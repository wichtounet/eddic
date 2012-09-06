//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <memory>
#include <thread>

#include "VisitorUtils.hpp"
#include "Options.hpp"
#include "PerfsTimer.hpp"
#include "iterators.hpp"
#include "likely.hpp"
#include "logging.hpp"

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

template<typename Visitor>
bool apply_to_all(std::shared_ptr<mtac::Function> function){
    Visitor visitor;

    mtac::visit_all_statements(visitor, function);

    return visitor.optimized;
}

template<typename Visitor>
bool apply_to_basic_blocks(std::shared_ptr<mtac::Function> function){
    bool optimized = false;

    for(auto& block : function->getBasicBlocks()){
        Visitor visitor;
        visit_each(visitor, block->statements);

        optimized |= visitor.optimized;
    }

    return optimized;
}

template<typename Visitor>
bool apply_to_basic_blocks_two_pass(std::shared_ptr<mtac::Function> function){
    bool optimized = false;

    for(auto& block : function->getBasicBlocks()){
        Visitor visitor;
        visitor.pass = mtac::Pass::DATA_MINING;

        visit_each(visitor, block->statements);

        visitor.pass = mtac::Pass::OPTIMIZE;

        visit_each(visitor, block->statements);

        optimized |= visitor.optimized;
    }

    return optimized;
}

template<typename Problem, typename... Args>
bool data_flow_optimization(std::shared_ptr<mtac::Function> function, Args... args){
    bool optimized = false;

    Problem problem(args...);

    auto results = mtac::data_flow(function, problem);

    //Once the data-flow problem is fixed, statements can be optimized
    for(auto& block : function->getBasicBlocks()){
        for(auto& statement : block->statements){
            optimized |= problem.optimize(statement, results);
        }
    }

    return optimized;
}

bool debug(const std::string& name, bool b, std::shared_ptr<mtac::Function> function){
    if(log::enabled<Dev>()){
        if(b){
            log::emit<Dev>("Optimizer") << "Optimization" << name << " returned true" << log::endl;

            //Print the function
            print(function);
        } else {
            log::emit<Dev>("Optimizer") << "Optimization" << name << " returned false" << log::endl;
        }
    }

    return b;
}

template<typename Functor, typename... Args>
bool debug(const std::string& name, Functor functor, std::shared_ptr<mtac::Function> function, Args... args){
    bool b;
    {
        PerfsTimer timer(name);

        b = functor(function, args...);
    }

    return debug(name, b, function);
}

void remove_nop(std::shared_ptr<mtac::Function> function){
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

void optimize_function(std::shared_ptr<mtac::Function> function, std::shared_ptr<StringPool> pool, Platform platform){
    if(log::enabled<Dev>()){
        log::emit<Dev>("Optimizer") << "Start optimizations on " << function->getName() << log::endl;

        print(function);
    }

    bool optimized;
    do {
        optimized = false;

        optimized |= debug("Aritmetic Identities", &apply_to_all<mtac::ArithmeticIdentities>, function);
        optimized |= debug("Reduce in Strength", &apply_to_all<mtac::ReduceInStrength>, function);
        optimized |= debug("Constant folding", &apply_to_all<mtac::ConstantFolding>, function);

        optimized |= debug("Constant propagation", &data_flow_optimization<mtac::ConstantPropagationProblem>, function);
        optimized |= debug("Offset Constant Propagation", &data_flow_optimization<mtac::OffsetConstantPropagationProblem, std::shared_ptr<StringPool>, Platform>, function, pool, platform);

        //If there was optimizations here, better to try again before perfoming common subexpression
        if(optimized){
            continue;
        }

        optimized |= debug("Common Subexpression Elimination", &data_flow_optimization<mtac::CommonSubexpressionElimination>, function);

        optimized |= debug("Pointer Propagation", &apply_to_basic_blocks<mtac::PointerPropagation>, function);
        optimized |= debug("Math Propagation", &apply_to_basic_blocks_two_pass<mtac::MathPropagation>, function);

        optimized |= debug("Optimize Branches", &mtac::optimize_branches, function);
        optimized |= debug("Optimize Concat", &mtac::optimize_concat, function, pool);
        optimized |= debug("Remove dead basic block", &mtac::remove_dead_basic_blocks, function);
        optimized |= debug("Merge basic block", &mtac::merge_basic_blocks, function);

        remove_nop(function);
        optimized |= debug("Dead-Code Elimination", &mtac::dead_code_elimination, function);
        
        optimized |= debug("Remove aliases", &mtac::remove_aliases, function);
    } while (optimized);

    //Remove variables that are not used after optimizations
    clean_variables(function);
}

void basic_optimize_function(std::shared_ptr<mtac::Function> function){
    if(log::enabled<Dev>()){
        log::emit<Dev>("Optimizer") << "Start basic optimizations on " << function->getName() << log::endl;

        print(function);
    }

    debug("Constant folding", apply_to_all<mtac::ConstantFolding>(function), function);
}

void optimize_all_functions(std::shared_ptr<mtac::Program> program, std::shared_ptr<StringPool> string_pool, Platform platform, std::shared_ptr<Configuration> configuration){
    PerfsTimer timer("Whole optimizations");

    auto& functions = program->functions;

    if(configuration->option_defined("dev")){
        for(auto& function : functions){
            optimize_function(function, string_pool, platform);
        }
    } else {
        //Find a better heuristic to configure the number of threads
        std::size_t threads = std::min(functions.size(), static_cast<std::size_t>(MAX_THREADS));

        std::vector<std::thread> pool;
        for(std::size_t tid = 0; tid < threads; ++tid){
            pool.push_back(std::thread([tid, threads, &string_pool, platform, &functions](){
                std::size_t i = tid;

                while(i < functions.size()){
                    optimize_function(functions[i], string_pool, platform); 

                    i += threads;
                }
            }));
        }

        //Wait for all the threads to finish
        std::for_each(pool.begin(), pool.end(), [](std::thread& thread){thread.join();});
    }
}

} //end of anonymous namespace

void mtac::Optimizer::optimize(std::shared_ptr<mtac::Program> program, std::shared_ptr<StringPool> string_pool, Platform platform, std::shared_ptr<Configuration> configuration) const {
    //Allocate storage for the temporaries that need to be stored
    allocate_temporary(program, platform);

    if(configuration->option_defined("fglobal-optimization")){
        bool optimized = false;
        do{
            mtac::remove_unused_functions(program);

            optimize_all_functions(program, string_pool, platform, configuration);

            optimized = mtac::remove_empty_functions(program);
            optimized = mtac::inline_functions(program, configuration);
        } while(optimized);
    } else {
        //Even if global optimizations are disabled, perform basic optimization (only constant folding)
        basic_optimize(program, string_pool, configuration);
    }
    
    //Allocate storage for the temporaries that need to be stored
    allocate_temporary(program, platform);
}

void mtac::Optimizer::basic_optimize(std::shared_ptr<mtac::Program> program, std::shared_ptr<StringPool> /*string_pool*/, std::shared_ptr<Configuration> configuration) const {
    PerfsTimer timer("Whole basic optimizations");

    auto& functions = program->functions;

    if(configuration->option_defined("dev")){
        for(auto& function : functions){
            basic_optimize_function(function); 
        }
    } else {
        //Find a better heuristic to configure the number of threads
        std::size_t threads = std::min(functions.size(), static_cast<std::size_t>(MAX_THREADS));

        std::vector<std::thread> pool;
        for(std::size_t tid = 0; tid < threads; ++tid){
            pool.push_back(std::thread([tid, threads, &functions](){
                std::size_t i = tid;

                while(i < functions.size()){
                    basic_optimize_function(functions[i]); 

                    i += threads;
                }
            }));
        }

        //Wait for all the threads to finish
        std::for_each(pool.begin(), pool.end(), [](std::thread& thread){thread.join();});
    }
}
