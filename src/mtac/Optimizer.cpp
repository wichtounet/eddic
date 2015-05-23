//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <memory>
#include <thread>
#include <type_traits>

#include "boost_cfg.hpp"
#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>

#include "cpp_utils/tmp.hpp"

#include "Options.hpp"
#include "PerfsTimer.hpp"
#include "iterators.hpp"
#include "logging.hpp"
#include "timing.hpp"
#include "GlobalContext.hpp"

#include "mtac/pass_traits.hpp"
#include "mtac/Utils.hpp"
#include "mtac/Pass.hpp"
#include "mtac/Optimizer.hpp"
#include "mtac/Program.hpp"
#include "mtac/ControlFlowGraph.hpp"
#include "mtac/Quadruple.hpp"

//The custom optimizations
#include "mtac/conditional_propagation.hpp"
#include "mtac/remove_aliases.hpp"
#include "mtac/clean_variables.hpp"
#include "mtac/remove_unused_functions.hpp"
#include "mtac/remove_empty_functions.hpp"
#include "mtac/DeadCodeElimination.hpp"
#include "mtac/merge_basic_blocks.hpp"
#include "mtac/remove_dead_basic_blocks.hpp"
#include "mtac/BranchOptimizations.hpp"
#include "mtac/inlining.hpp"
#include "mtac/loop_analysis.hpp"
#include "mtac/induction_variable_optimizations.hpp"
#include "mtac/loop_unrolling.hpp"
#include "mtac/loop_unswitching.hpp"
#include "mtac/complete_loop_peeling.hpp"
#include "mtac/remove_empty_loops.hpp"
#include "mtac/loop_invariant_code_motion.hpp"
#include "mtac/parameter_propagation.hpp"
#include "mtac/pure_analysis.hpp"
#include "mtac/local_cse.hpp"

//The optimization visitors
#include "mtac/ArithmeticIdentities.hpp"
#include "mtac/ReduceInStrength.hpp"
#include "mtac/ConstantFolding.hpp"
#include "mtac/MathPropagation.hpp"
#include "mtac/PointerPropagation.hpp"

//The data-flow problems
#include "mtac/GlobalOptimizations.hpp"
#include "mtac/global_cp.hpp"
#include "mtac/global_offset_cp.hpp"
#include "mtac/global_cse.hpp"

#include "ltac/Register.hpp"
#include "ltac/FloatRegister.hpp"
#include "ltac/PseudoRegister.hpp"
#include "ltac/PseudoFloatRegister.hpp"
#include "ltac/Address.hpp"

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
        mtac::conditional_propagation*,
        mtac::ConstantPropagationProblem*,
        mtac::OffsetConstantPropagationProblem*,
        mtac::local_cse*,
        mtac::global_cse*,
        mtac::PointerPropagation*,
        mtac::MathPropagation*,
        mtac::optimize_branches*,
        mtac::remove_dead_basic_blocks*,
        mtac::merge_basic_blocks*,
        mtac::dead_code_elimination*,
        mtac::remove_aliases*,
        mtac::loop_analysis*,
        mtac::loop_invariant_code_motion*,
        mtac::loop_induction_variables_optimization*,
        mtac::remove_empty_loops*,
        mtac::loop_unrolling*,
        mtac::loop_unswitching*,
        mtac::complete_loop_peeling*, //Must be kept last since it can mess up the loop analysis
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

template<typename T, typename Sign>
struct has_gate {
    typedef char yes[1];
    typedef char no [2];
    template <typename U, U> struct type_check;
    template <typename _1> static yes &chk(type_check<Sign, &_1::gate> *);
    template <typename   > static no  &chk(...);
    static bool const value = sizeof(chk<T>(0)) == sizeof(yes);
};

typedef boost::mpl::vector<
        mtac::remove_unused_functions*,
        mtac::all_basic_optimizations*
    > ipa_basic_passes;

typedef boost::mpl::vector<
        mtac::remove_unused_functions*,
        mtac::pure_analysis*,
        mtac::all_optimizations*,
        mtac::remove_empty_functions*,
        mtac::inline_functions*,
        mtac::remove_unused_functions*,
        mtac::parameter_propagation*
    > ipa_passes;

template<typename Pass>
struct need_pool {
    static const bool value = mtac::pass_traits<Pass>::property_flags & mtac::PROPERTY_POOL;
};

template<typename Pass>
struct need_platform {
    static const bool value = mtac::pass_traits<Pass>::property_flags & mtac::PROPERTY_PLATFORM;
};

template<typename Pass>
struct need_configuration {
    static const bool value = mtac::pass_traits<Pass>::property_flags & mtac::PROPERTY_CONFIGURATION;
};

template<typename Pass>
struct need_program {
    static const bool value = mtac::pass_traits<Pass>::property_flags & mtac::PROPERTY_PROGRAM;
};

struct pass_runner {
    bool optimized = false;

    mtac::Program& program;
    mtac::Function* function;

    std::shared_ptr<StringPool> pool;
    std::shared_ptr<Configuration> configuration;
    Platform platform;
    timing_system& system;

    pass_runner(mtac::Program& program, std::shared_ptr<StringPool> pool, std::shared_ptr<Configuration> configuration, Platform platform, timing_system& system) :
            program(program), pool(pool), configuration(configuration), platform(platform), system(system) {};

    template<typename Pass>
    inline void apply_todo(){
        //No todo are implemented
    }

    template<typename Pass>
    inline typename std::enable_if_t<need_pool<Pass>::value, void> set_pool(Pass& pass){
        pass.set_pool(pool);
    }

    template<typename Pass>
    inline typename std::enable_if_t<!need_pool<Pass>::value, void> set_pool(Pass&){
        //NOP
    }

    template<typename Pass>
    inline typename std::enable_if_t<need_platform<Pass>::value, void> set_platform(Pass& pass){
        pass.set_platform(platform);
    }

    template<typename Pass>
    inline typename std::enable_if_t<!need_platform<Pass>::value, void> set_platform(Pass&){
        //NOP
    }

    template<typename Pass>
    inline typename std::enable_if_t<need_configuration<Pass>::value, void> set_configuration(Pass& pass){
        pass.set_configuration(configuration);
    }

    template<typename Pass>
    inline typename std::enable_if_t<!need_configuration<Pass>::value, void> set_configuration(Pass&){
        //NOP
    }

    template<typename Pass>
    inline typename std::enable_if_t<need_program<Pass>::value, Pass> construct(){
        return Pass(program);
    }

    template<typename Pass>
    inline typename std::enable_if_t<!need_program<Pass>::value, Pass> construct(){
        return Pass();
    }

    template<typename Pass>
    Pass make_pass(){
        auto pass = construct<Pass>();

        set_pool(pass);
        set_platform(pass);
        set_configuration(pass);

        return pass;
    }

    template<typename Pass>
    inline typename std::enable_if_t<has_gate<Pass, bool(Pass::*)(std::shared_ptr<Configuration>)>::value, bool> has_to_be_run(Pass& pass){
        return pass.gate(configuration);
    }

    template<typename Pass>
    inline typename std::enable_if_t<!has_gate<Pass, bool(Pass::*)(std::shared_ptr<Configuration>)>::value, bool> has_to_be_run(Pass&){
        return true;
    }

    template<typename Pass>
    inline typename std::enable_if<mtac::pass_traits<Pass>::type == mtac::pass_type::IPA, bool>::type apply(Pass& pass){
        return pass(program);
    }

    template<typename Pass>
    inline typename std::enable_if<mtac::pass_traits<Pass>::type == mtac::pass_type::IPA_SUB, bool>::type apply(Pass&){
        for(auto& function : program.functions){
            this->function = &function;

            if(log::enabled<Debug>()){
                LOG<Debug>("Optimizer") << "Start optimizations on " << function.get_name() << log::endl;

                std::cout << function << std::endl;
            }

            boost::mpl::for_each<typename mtac::pass_traits<Pass>::sub_passes>(boost::ref(*this));
        }

        return false;
    }

    template<typename Pass>
    inline typename std::enable_if<mtac::pass_traits<Pass>::type == mtac::pass_type::CUSTOM, bool>::type apply(Pass& pass){
        return pass(*function);
    }

    template<typename Pass>
    inline typename std::enable_if<mtac::pass_traits<Pass>::type == mtac::pass_type::LOCAL, bool>::type apply(Pass& visitor){
        for(auto& block : *function){
            for(auto& quadruple : block->statements){
                visitor(quadruple);
            }
        }

        return visitor.optimized;
    }

    template<typename Pass>
    inline typename std::enable_if<mtac::pass_traits<Pass>::type == mtac::pass_type::DATA_FLOW, bool>::type apply(Pass& problem){
        auto results = mtac::data_flow(*function, problem);

        //Once the data-flow problem is fixed, statements can be optimized
        return problem.optimize(*function, results);
    }

    template<typename Pass>
    inline typename std::enable_if<mtac::pass_traits<Pass>::type == mtac::pass_type::BB, bool>::type apply(Pass& visitor){
        bool optimized = false;

        for(auto& block : *function){
            visitor.clear();

            for(auto& quadruple : block->statements){
                visitor(quadruple);
            }

            optimized |= visitor.optimized;
        }

        return optimized;
    }

    template<typename Pass>
    inline typename std::enable_if<mtac::pass_traits<Pass>::type == mtac::pass_type::BB_TWO_PASS, bool>::type apply(Pass& visitor){
        bool optimized = false;

        for(auto& block : *function){
            visitor.clear();

            visitor.pass = mtac::Pass::DATA_MINING;
            for(auto& quadruple : block->statements){
                visitor(quadruple);
            }

            visitor.pass = mtac::Pass::OPTIMIZE;
            for(auto& quadruple : block->statements){
                visitor(quadruple);
            }

            optimized |= visitor.optimized;
        }

        return optimized;
    }

    template<typename Pass>
    inline typename std::enable_if<cpp::or_u<mtac::pass_traits<Pass>::type == mtac::pass_type::IPA, mtac::pass_traits<Pass>::type == mtac::pass_type::IPA_SUB>::value, void>::type
    debug_local(bool local){
        LOG<Debug>("Optimizer") << mtac::pass_traits<Pass>::name() << " returned " << local << log::endl;
    }

    template<typename Pass>
    inline typename std::enable_if<cpp::and_u<mtac::pass_traits<Pass>::type != mtac::pass_type::IPA, mtac::pass_traits<Pass>::type != mtac::pass_type::IPA_SUB>::value, void>::type
    debug_local(bool local){
        if(log::enabled<Debug>()){
            if(local){
                LOG<Debug>("Optimizer") << mtac::pass_traits<Pass>::name() << " returned true" << log::endl;

                //Print the function
                std::cout << *function << std::endl;
            } else {
                LOG<Debug>("Optimizer") << mtac::pass_traits<Pass>::name() << " returned false" << log::endl;
            }
        }
    }

    template<typename Pass>
    inline void operator()(Pass*){
        auto pass = make_pass<Pass>();

        if(has_to_be_run(pass)){
            timing_timer timer(system, mtac::pass_traits<Pass>::name());

            bool local = local = apply<Pass>(pass);
            if(local){
                program.context->stats().inc_counter(std::string(mtac::pass_traits<Pass>::name()) + "_true");
                apply_todo<Pass>();
            }

            debug_local<Pass>(local);

            optimized |= local;
        }
    }
};

} //end of anonymous namespace

void mtac::Optimizer::optimize(mtac::Program& program, std::shared_ptr<StringPool> string_pool, Platform platform, std::shared_ptr<Configuration> configuration) const {
    timing_timer timer(program.context->timing(), "whole_optimizations");

    //Build the CFG of each functions (also needed for register allocation)
    for(auto& function : program.functions){
        timing_timer timer(program.context->timing(), "build_cfg");
        mtac::build_control_flow_graph(function);
    }

    if(configuration->option_defined("fglobal-optimization")){
        //Apply Interprocedural Optimizations
        pass_runner runner(program, string_pool, configuration, platform, program.context->timing());
        do{
            runner.optimized = false;
            boost::mpl::for_each<ipa_passes>(boost::ref(runner));
        } while(runner.optimized);
    } else {
        //Even if global optimizations are disabled, perform basic optimization (only constant folding)
        pass_runner runner(program, string_pool, configuration, platform, program.context->timing());
        boost::mpl::for_each<ipa_basic_passes>(boost::ref(runner));
    }
}
