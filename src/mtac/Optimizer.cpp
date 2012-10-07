//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <memory>
#include <thread>

#include "boost_cfg.hpp"
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>

#include "VisitorUtils.hpp"
#include "Options.hpp"
#include "PerfsTimer.hpp"
#include "iterators.hpp"
#include "likely.hpp"
#include "logging.hpp"

#include "mtac/pass_traits.hpp"
#include "mtac/Utils.hpp"
#include "mtac/Pass.hpp"
#include "mtac/Optimizer.hpp"
#include "mtac/Program.hpp"
#include "mtac/Printer.hpp"
#include "mtac/TemporaryAllocator.hpp"

//The custom optimizations
#include "mtac/VariableOptimizations.hpp"
#include "mtac/FunctionOptimizations.hpp"
#include "mtac/DeadCodeElimination.hpp"
#include "mtac/BasicBlockOptimizations.hpp"
#include "mtac/BranchOptimizations.hpp"
#include "mtac/ConcatReduction.hpp"
#include "mtac/inlining.hpp"
#include "mtac/loop_optimizations.hpp"

//The optimization visitors
#include "mtac/ArithmeticIdentities.hpp"
#include "mtac/ReduceInStrength.hpp"
#include "mtac/ConstantFolding.hpp"
#include "mtac/RemoveAssign.hpp"
#include "mtac/RemoveMultipleAssign.hpp"
#include "mtac/MathPropagation.hpp"
#include "mtac/PointerPropagation.hpp"

//The data-flow problems
#include "mtac/GlobalOptimizations.hpp"
#include "mtac/ConstantPropagationProblem.hpp"
#include "mtac/OffsetConstantPropagationProblem.hpp"
#include "mtac/CommonSubexpressionElimination.hpp"

using namespace eddic;

namespace {

const unsigned int MAX_THREADS = 2;

//TODO Find a more elegant way than using pointers

typedef boost::mpl::vector<
        mtac::ConstantFolding* 
    > basic_passes;

typedef boost::mpl::vector<
        mtac::ArithmeticIdentities*, 
        mtac::ReduceInStrength*, 
        mtac::ConstantFolding*, 
        mtac::ConstantPropagationProblem*,
        mtac::OffsetConstantPropagationProblem*,
        mtac::CommonSubexpressionElimination*,
        mtac::PointerPropagation*,
        mtac::MathPropagation*,
        mtac::optimize_branches*,
        mtac::optimize_concat*,
        mtac::remove_dead_basic_blocks*,
        mtac::merge_basic_blocks*,
        mtac::dead_code_elimination*,
        mtac::remove_aliases*,
        mtac::loop_invariant_code_motion*,
        mtac::loop_induction_variables_optimization*,
        mtac::remove_empty_loops*,
        mtac::clean_variables*
    > passes;

struct pass_runner {
    bool optimized = false;
    std::shared_ptr<mtac::Function> function;
    std::shared_ptr<StringPool> pool;
    Platform platform;

    pass_runner(std::shared_ptr<mtac::Function> function, std::shared_ptr<StringPool> pool, Platform platform) : 
            function(function), pool(pool), platform(platform) {};

    template<typename Pass>
    inline typename boost::enable_if_c<mtac::pass_traits<Pass>::todo_flags & mtac::TODO_REMOVE_NOP, void>::type remove_nop(){
        for(auto& block : function->getBasicBlocks()){
            auto it = iterate(block->statements);

            while(it.has_next()){
                if(unlikely(boost::get<std::shared_ptr<mtac::NoOp>>(&*it))){
                    it.erase();
                    continue;
                } else if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&*it)){
                    if((*ptr)->op == mtac::Operator::NOP){
                        it.erase();
                        continue;
                    }
                }

                ++it;
            }
        }
    }
    
    template<typename Pass>
    inline typename boost::disable_if_c<mtac::pass_traits<Pass>::todo_flags & mtac::TODO_REMOVE_NOP, void>::type remove_nop(){
        //NOP
    }
    
    template<typename Pass>
    inline void apply_todo(){
        remove_nop<Pass>();
    }

    template<typename Pass>
    inline typename boost::enable_if_c<mtac::pass_traits<Pass>::need_pool, void>::type set_pool(Pass& pass){
        pass.set_pool(pool);
    }
    
    template<typename Pass>
    inline typename boost::disable_if_c<mtac::pass_traits<Pass>::need_pool, void>::type set_pool(Pass&){
        //NOP
    }
    
    template<typename Pass>
    inline typename boost::enable_if_c<mtac::pass_traits<Pass>::need_platform, void>::type set_platform(Pass& pass){
        pass.set_platform(platform);
    }
    
    template<typename Pass>
    inline typename boost::disable_if_c<mtac::pass_traits<Pass>::need_platform, void>::type set_platform(Pass&){
        //NOP
    }

    template<typename Pass>
    Pass make_pass(){
        Pass pass;

        set_pool(pass);
        set_platform(pass);

        return pass;
    }
    
    template<typename Pass>
    inline typename boost::enable_if_c<mtac::pass_traits<Pass>::type == mtac::pass_type::CUSTOM, bool>::type apply(){
        auto pass = make_pass<Pass>();

        return pass(function);
    }

    template<typename Pass>
    inline typename boost::enable_if_c<mtac::pass_traits<Pass>::type == mtac::pass_type::LOCAL, bool>::type apply(){
        auto visitor = make_pass<Pass>();

        mtac::visit_all_statements(visitor, function);

        return visitor.optimized;
    }
    
    template<typename Pass>
    inline typename boost::enable_if_c<mtac::pass_traits<Pass>::type == mtac::pass_type::DATA_FLOW, bool>::type apply(){
        bool optimized = false;

        auto problem = make_pass<Pass>();

        auto results = mtac::data_flow(function, problem);

        //Once the data-flow problem is fixed, statements can be optimized
        for(auto& block : function->getBasicBlocks()){
            for(auto& statement : block->statements){
                optimized |= problem.optimize(statement, results);
            }
        }

        return optimized;
    }
    
    template<typename Pass>
    inline typename boost::enable_if_c<mtac::pass_traits<Pass>::type == mtac::pass_type::BB, bool>::type apply(){
        bool optimized = false;
        
        auto visitor = make_pass<Pass>();

        for(auto& block : function->getBasicBlocks()){
            visitor.clear();

            visit_each(visitor, block->statements);

            optimized |= visitor.optimized;
        }

        return optimized;
    }
    
    template<typename Pass>
    inline typename boost::enable_if_c<mtac::pass_traits<Pass>::type == mtac::pass_type::BB_TWO_PASS, bool>::type apply(){
        bool optimized = false;
        
        auto visitor = make_pass<Pass>();

        for(auto& block : function->getBasicBlocks()){
            visitor.clear();

            visitor.pass = mtac::Pass::DATA_MINING;
            visit_each(visitor, block->statements);

            visitor.pass = mtac::Pass::OPTIMIZE;
            visit_each(visitor, block->statements);

            optimized |= visitor.optimized;
        }

        return optimized;
    }

    template<typename Pass>
    inline void operator()(Pass*){
        bool local = false;
        {
            PerfsTimer timer(mtac::pass_traits<Pass>::name());

            local = apply<Pass>();
        }

        apply_todo<Pass>();
    
        if(log::enabled<Debug>()){
            if(local){
                log::emit<Debug>("Optimizer") << mtac::pass_traits<Pass>::name() << " returned true" << log::endl;

                //Print the function
                print(function);
            } else {
                log::emit<Debug>("Optimizer") << mtac::pass_traits<Pass>::name() << " returned false" << log::endl;
            }
        }

        optimized = local;
    }
};

template<typename Passes>
void optimize_function(std::shared_ptr<mtac::Function> function, std::shared_ptr<StringPool> pool, Platform platform){
    if(log::enabled<Debug>()){
        log::emit<Debug>("Optimizer") << "Start optimizations on " << function->getName() << log::endl;

        print(function);
    }
    
    pass_runner runner(function, pool, platform);
    do{
        boost::mpl::for_each<Passes>(runner);
    } while(runner.optimized);
}

} //end of anonymous namespace

void mtac::Optimizer::optimize(std::shared_ptr<mtac::Program> program, std::shared_ptr<StringPool> string_pool, Platform platform, std::shared_ptr<Configuration> configuration) const {
    PerfsTimer timer("Whole optimizations");

    //Allocate storage for the temporaries that need to be stored
    allocate_temporary(program, platform);

    if(configuration->option_defined("fglobal-optimization")){
        auto& functions = program->functions;
        for(auto& function : functions){
            optimize_function<passes>(function, string_pool, platform);
        }
    } else {
        //Even if global optimizations are disabled, perform basic optimization (only constant folding)
        for(auto& function : program->functions){
            optimize_function<basic_passes>(function, string_pool, platform); 
        }
    }
    
    //Allocate storage for the temporaries that need to be stored
    allocate_temporary(program, platform);
}
