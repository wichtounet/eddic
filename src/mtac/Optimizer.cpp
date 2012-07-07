//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <memory>
#include <thread>

//#include "Utils.hpp"
#include "VisitorUtils.hpp"
#include "Options.hpp"
#include "PerfsTimer.hpp"
#include "likely.hpp"

#include "mtac/Pass.hpp"
#include "mtac/Optimizer.hpp"
#include "mtac/Program.hpp"
#include "mtac/Printer.hpp"

//The custom optimizations
#include "mtac/VariableCleaner.hpp"
#include "mtac/RemoveEmptyFunctions.hpp"
#include "mtac/DeadCodeElimination.hpp"
#include "mtac/BasicBlockOptimizations.hpp"
#include "mtac/ConcatReduction.hpp"

//The optimization visitors
#include "mtac/ArithmeticIdentities.hpp"
#include "mtac/ReduceInStrength.hpp"
#include "mtac/ConstantFolding.hpp"
#include "mtac/RemoveAssign.hpp"
#include "mtac/RemoveMultipleAssign.hpp"
#include "mtac/MathPropagation.hpp"

//The data-flow problems
#include "mtac/GlobalOptimizations.hpp"
#include "mtac/ConstantPropagationProblem.hpp"
#include "mtac/OffsetConstantPropagationProblem.hpp"
#include "mtac/CommonSubexpressionElimination.hpp"

using namespace eddic;

namespace {

static const unsigned int MAX_THREADS = 2;
static const bool DebugPerf = false;

template<typename Visitor>
bool apply_to_all(std::shared_ptr<mtac::Function> function){
    Visitor visitor;

    for(auto& block : function->getBasicBlocks()){
        visit_each(visitor, block->statements);
    }

    return visitor.optimized;
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


bool optimize_branches(std::shared_ptr<mtac::Function> function){
    bool optimized = false;
    
    for(auto& block : function->getBasicBlocks()){
        for(auto& statement : block->statements){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::IfFalse>>(&statement)){
                if(!(*ptr)->op && boost::get<int>(&(*ptr)->arg1)){
                    int value = boost::get<int>((*ptr)->arg1);

                    if(value == 0){
                        auto goto_ = std::make_shared<mtac::Goto>();

                        goto_->label = (*ptr)->label;
                        goto_->block = (*ptr)->block;

                        statement = goto_;
                        optimized = true;
                    } else if(value == 1){
                        statement = std::make_shared<mtac::NoOp>();
                        optimized = true;
                    }
                }
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::If>>(&statement)){
                if(!(*ptr)->op && boost::get<int>(&(*ptr)->arg1)){
                    int value = boost::get<int>((*ptr)->arg1);

                    if(value == 0){
                        statement = std::make_shared<mtac::NoOp>();
                        optimized = true;
                    } else if(value == 1){
                        auto goto_ = std::make_shared<mtac::Goto>();

                        goto_->label = (*ptr)->label;
                        goto_->block = (*ptr)->block;

                        statement = goto_;
                        optimized = true;
                    }
                }
            }
        }
    }

    return optimized;
}

bool remove_needless_jumps(std::shared_ptr<mtac::Function> function){
    bool optimized = false;

    auto& blocks = function->getBasicBlocks();

    auto it = blocks.begin();
    auto end = blocks.end();

    while(it != end){
        auto& block = *it;

        if(likely(!block->statements.empty())){
            auto& last = block->statements[block->statements.size() - 1];

            if(auto* ptr = boost::get<std::shared_ptr<mtac::Goto>>(&last)){
                auto next = it;
                ++next;

                //If the target block is the next in the list 
                if((*ptr)->block == *next){
                    block->statements.pop_back();

                    optimized = true;
                }
            }
        }

        ++it;
    }

    return optimized;
}

template<typename Problem>
bool data_flow_optimization(std::shared_ptr<mtac::Function> function){
    bool optimized = false;

    Problem problem;

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
    if(option_defined("dev")){
        if(b){
            std::cout << "optimization " << name << " returned true" << std::endl;

            //Print the function
            print(function);
        } else {
            std::cout << "optimization " << name << " returned false" << std::endl;
        }
    }

    return b;
}

}

void optimize_function(std::shared_ptr<mtac::Function> function, std::shared_ptr<StringPool> pool){
    if(option_defined("dev")){
        std::cout << "Start optimizations on " << function->getName() << std::endl;

        print(function);
    }

    bool optimized;
    do {
        optimized = false;

        optimized |= debug("Aritmetic Identities", apply_to_all<mtac::ArithmeticIdentities>(function), function);
        optimized |= debug("Reduce in Strength", apply_to_all<mtac::ReduceInStrength>(function), function);
        optimized |= debug("Constant folding", apply_to_all<mtac::ConstantFolding>(function), function);

        optimized |= debug("Constant propagation", data_flow_optimization<mtac::ConstantPropagationProblem>(function), function);
        optimized |= debug("Offset Constant Propagation", data_flow_optimization<mtac::OffsetConstantPropagationProblem>(function), function);

        //If there was optimizations here, better to try again before perfoming common subexpression
        if(optimized){
            continue;
        }

        optimized |= debug("Common Subexpression Elimination", data_flow_optimization<mtac::CommonSubexpressionElimination>(function), function);

        optimized |= debug("Math Propagation", apply_to_basic_blocks_two_pass<mtac::MathPropagation>(function), function);
        
        optimized |= debug("Dead-Code Elimination", mtac::dead_code_elimination(function), function);

        optimized |= debug("Optimize Branches", optimize_branches(function), function);
        optimized |= debug("Optimize Concat", optimize_concat(function, pool), function);
        optimized |= debug("Remove dead basic block", remove_dead_basic_blocks(function), function);
        optimized |= debug("Remove needless jumps", remove_needless_jumps(function), function);
    } while (optimized);

    //Remove variables that are not used after optimizations
    clean_variables(function);
}

void basic_optimize_function(std::shared_ptr<mtac::Function> function){
    if(option_defined("dev")){
        std::cout << "Start basic optimizations on " << function->getName() << std::endl;

        print(function);
    }

    debug("Constant folding", apply_to_all<mtac::ConstantFolding>(function), function);
    
    /*//Liveness debugging
    mtac::LiveVariableAnalysisProblem problem;
    auto results = mtac::data_flow(function, problem);

    for(auto& block : function->getBasicBlocks()){
        auto it = block->statements.begin();
        auto end = block->statements.end();

        while(it != end){
            auto statement = *it;

            mtac::Printer printer;
            printer.printStatement(statement);
            std::cout << "OUT{";
            for(auto& var : results->OUT_S[statement].values()){
                std::cout << var->name() << ", ";
            }
            std::cout << "}" << std::endl;
            std::cout << "IN{";
            for(auto& var : results->IN_S[statement].values()){
                std::cout << var->name() << ", ";
            }
            std::cout << "}" << std::endl;

            ++it;
        }
    }*/
}

void optimize_all_functions(std::shared_ptr<mtac::Program> program, std::shared_ptr<StringPool> string_pool){
    PerfsTimer timer("Whole optimizations");

    auto& functions = program->functions;

    //Find a better heuristic to configure the number of threads
    std::size_t threads = std::min(functions.size(), static_cast<std::size_t>(MAX_THREADS));

    if(option_defined("dev")){
        threads = 1;
    }

    std::vector<std::thread> pool;
    for(std::size_t tid = 0; tid < threads; ++tid){
        pool.push_back(std::thread([tid, threads, &string_pool, &functions](){
            std::size_t i = tid;

            while(i < functions.size()){
                optimize_function(functions[i], string_pool); 

                i += threads;
            }
        }));
    }

    //Wait for all the threads to finish
    std::for_each(pool.begin(), pool.end(), [](std::thread& thread){thread.join();});
}

void mtac::Optimizer::optimize(std::shared_ptr<mtac::Program> program, std::shared_ptr<StringPool> string_pool) const {
    bool optimized = false;

    do{
        optimize_all_functions(program, string_pool);

        optimized = mtac::remove_empty_functions(program);
    } while(optimized);
}

void mtac::Optimizer::basic_optimize(std::shared_ptr<mtac::Program> program, std::shared_ptr<StringPool> /*string_pool*/) const {
    PerfsTimer timer("Whole basic optimizations");

    auto& functions = program->functions;

    //Find a better heuristic to configure the number of threads
    std::size_t threads = std::min(functions.size(), static_cast<std::size_t>(MAX_THREADS));

    if(option_defined("dev")){
        threads = 1;
    }

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
