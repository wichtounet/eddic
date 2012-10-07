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

namespace eddic {
namespace mtac {

typedef boost::mpl::vector<
        mtac::ConstantFolding*
    > basic_passes;

struct all_basic_optimizations {};

template<>
struct pass_traits<all_basic_optimizations> {
    STATIC_CONSTANT(pass_type, type, pass_type::IPA_SUB);
    STATIC_STRING(name, "all_basic_optimizations");
    STATIC_CONSTANT(unsigned int, property_flags, 0);
    STATIC_CONSTANT(unsigned int, todo_after_flags, 0);

    typedef basic_passes sub_passes;
};

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

struct all_optimizations {};

template<>
struct pass_traits<all_optimizations> {
    STATIC_CONSTANT(pass_type, type, pass_type::IPA_SUB);
    STATIC_STRING(name, "all_optimizations");
    STATIC_CONSTANT(unsigned int, property_flags, 0);
    STATIC_CONSTANT(unsigned int, todo_after_flags, 0);

    typedef passes sub_passes;
};

}
}

namespace {

//TODO Find a more elegant way than using pointers

typedef boost::mpl::vector<
        mtac::all_basic_optimizations*,
        mtac::allocate_temporary*
    > ipa_basic_passes;

typedef boost::mpl::vector<
        mtac::allocate_temporary*,
        mtac::remove_unused_functions*,
        mtac::all_optimizations*,
        mtac::remove_empty_functions*,
        mtac::inline_functions*
    > ipa_passes;

struct pass_runner {
    bool optimized = false;
    
    std::shared_ptr<mtac::Program> program;
    std::shared_ptr<mtac::Function> function;

    std::shared_ptr<StringPool> pool;
    std::shared_ptr<Configuration> configuration;
    Platform platform;

    pass_runner(std::shared_ptr<mtac::Program> program, std::shared_ptr<StringPool> pool, std::shared_ptr<Configuration> configuration, Platform platform) : 
            program(program), pool(pool), configuration(configuration), platform(platform) {};

    template<typename Pass>
    inline typename boost::enable_if_c<mtac::pass_traits<Pass>::todo_after_flags & mtac::TODO_REMOVE_NOP, void>::type remove_nop(){
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
    inline typename boost::disable_if_c<mtac::pass_traits<Pass>::todo_after_flags & mtac::TODO_REMOVE_NOP, void>::type remove_nop(){
        //NOP
    }
    
    template<typename Pass>
    inline void apply_todo(){
        remove_nop<Pass>();
    }

    template<typename Pass>
    inline typename boost::enable_if_c<mtac::pass_traits<Pass>::property_flags & mtac::PROPERTY_POOL, void>::type set_pool(Pass& pass){
        pass.set_pool(pool);
    }
    
    template<typename Pass>
    inline typename boost::disable_if_c<mtac::pass_traits<Pass>::property_flags & mtac::PROPERTY_POOL, void>::type set_pool(Pass&){
        //NOP
    }
    
    template<typename Pass>
    inline typename boost::enable_if_c<mtac::pass_traits<Pass>::property_flags & mtac::PROPERTY_PLATFORM, void>::type set_platform(Pass& pass){
        pass.set_platform(platform);
    }
    
    template<typename Pass>
    inline typename boost::disable_if_c<mtac::pass_traits<Pass>::property_flags & mtac::PROPERTY_PLATFORM, void>::type set_platform(Pass&){
        //NOP
    }
    
    template<typename Pass>
    inline typename boost::enable_if_c<mtac::pass_traits<Pass>::property_flags & mtac::PROPERTY_CONFIGURATION, void>::type set_configuration(Pass& pass){
        pass.set_configuration(configuration);
    }
    
    template<typename Pass>
    inline typename boost::disable_if_c<mtac::pass_traits<Pass>::property_flags & mtac::PROPERTY_CONFIGURATION, void>::type set_configuration(Pass&){
        //NOP
    }

    template<typename Pass>
    Pass make_pass(){
        Pass pass;

        set_pool(pass);
        set_platform(pass);
        set_configuration(pass);

        return pass;
    }
    
    template<typename Pass>
    inline typename boost::enable_if_c<mtac::pass_traits<Pass>::type == mtac::pass_type::IPA, bool>::type apply(){
        auto pass = make_pass<Pass>();

        return pass(program);
    }
    
    template<typename Pass>
    inline typename boost::enable_if_c<mtac::pass_traits<Pass>::type == mtac::pass_type::IPA_SUB, bool>::type apply(){
        auto& functions = program->functions;
        for(auto& function : functions){
            this->function = function;
    
            if(log::enabled<Debug>()){
                log::emit<Debug>("Optimizer") << "Start optimizations on " << function->getName() << log::endl;

                print(function);
            }

            boost::mpl::for_each<typename mtac::pass_traits<Pass>::sub_passes>(boost::ref(*this));
        }

        return false;
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
    inline typename boost::enable_if_c<boost::type_traits::ice_or<mtac::pass_traits<Pass>::type == mtac::pass_type::IPA, mtac::pass_traits<Pass>::type == mtac::pass_type::IPA_SUB>::value, void>::type 
    debug_local(bool local){
        log::emit<Debug>("Optimizer") << mtac::pass_traits<Pass>::name() << " returned " << local << log::endl;
    }

    template<typename Pass>
    inline typename boost::enable_if_c<boost::type_traits::ice_and<mtac::pass_traits<Pass>::type != mtac::pass_type::IPA, mtac::pass_traits<Pass>::type != mtac::pass_type::IPA_SUB>::value, void>::type 
    debug_local(bool local){
        if(log::enabled<Debug>()){
            if(local){
                log::emit<Debug>("Optimizer") << mtac::pass_traits<Pass>::name() << " returned true" << log::endl;

                //Print the function
                print(function);
            } else {
                log::emit<Debug>("Optimizer") << mtac::pass_traits<Pass>::name() << " returned false" << log::endl;
            }
        }
    }

    template<typename Pass>
    inline void operator()(Pass*){
        bool local = false;
        {
            PerfsTimer timer(mtac::pass_traits<Pass>::name());

            local = apply<Pass>();
        }

        apply_todo<Pass>();

        debug_local<Pass>(local);

        optimized |= local;
    }
};

} //end of anonymous namespace

void mtac::Optimizer::optimize(std::shared_ptr<mtac::Program> program, std::shared_ptr<StringPool> string_pool, Platform platform, std::shared_ptr<Configuration> configuration) const {
    PerfsTimer timer("Whole optimizations");

    if(configuration->option_defined("fglobal-optimization")){
        //Apply Interprocedural Optimizations
        pass_runner runner(program, string_pool, configuration, platform);
        do{
            runner.optimized = false;
            boost::mpl::for_each<ipa_passes>(boost::ref(runner));
        } while(runner.optimized);
    } else {
        //Even if global optimizations are disabled, perform basic optimization (only constant folding and temporary cleaning)
        pass_runner runner(program, string_pool, configuration, platform);
        boost::mpl::for_each<ipa_basic_passes>(boost::ref(runner));
    }
}
