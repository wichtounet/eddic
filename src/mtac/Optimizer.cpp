//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <memory>
#include <unordered_set>
#include <thread>
#include <algorithm>

#include <boost/variant.hpp>
#include <boost/utility/enable_if.hpp>

#include "Utils.hpp"
#include "VisitorUtils.hpp"
#include "StringPool.hpp"
#include "Options.hpp"
#include "PerfsTimer.hpp"
#include "likely.hpp"

#include "mtac/Pass.hpp"
#include "mtac/Optimizer.hpp"
#include "mtac/Program.hpp"
#include "mtac/Utils.hpp"
#include "mtac/Printer.hpp"

//The data-flow problems
#include "mtac/GlobalOptimizations.hpp"
#include "mtac/ConstantPropagationProblem.hpp"
#include "mtac/OffsetConstantPropagationProblem.hpp"
#include "mtac/CommonSubexpressionElimination.hpp"
#include "mtac/LiveVariableAnalysisProblem.hpp"

//The optimization visitors
#include "mtac/ArithmeticIdentities.hpp"
#include "mtac/ReduceInStrength.hpp"
#include "mtac/ConstantFolding.hpp"
#include "mtac/RemoveAssign.hpp"
#include "mtac/RemoveMultipleAssign.hpp"
#include "mtac/MathPropagation.hpp"

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

bool remove_dead_basic_blocks(std::shared_ptr<mtac::Function> function){
    std::unordered_set<std::shared_ptr<mtac::BasicBlock>> usage;

    auto& blocks = function->getBasicBlocks();

    unsigned int before = blocks.size();

    auto it = blocks.begin();
    auto end = blocks.end();

    while(it != end){
        auto& block = *it;

        usage.insert(block);

        if(likely(!block->statements.empty())){
            auto& last = block->statements[block->statements.size() - 1];

            if(auto* ptr = boost::get<std::shared_ptr<mtac::Goto>>(&last)){
                if(usage.find((*ptr)->block) == usage.end()){
                    it = std::find(blocks.begin(), blocks.end(), (*ptr)->block);
                    continue;
                }
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::IfFalse>>(&last)){
                usage.insert((*ptr)->block); 
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::If>>(&last)){
                usage.insert((*ptr)->block); 
            }
        }

        ++it;
    }

    //The ENTRY and EXIT blocks should not be removed
    usage.insert(blocks.front());
    usage.insert(blocks.back());

    it = blocks.begin();
    end = blocks.end();

    blocks.erase(
            std::remove_if(it, end, 
                [&](std::shared_ptr<mtac::BasicBlock>& b){ return usage.find(b) == usage.end(); }), 
            end);

    return blocks.size() < before;
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

template<typename T>
bool isParam(T& statement){
    return boost::get<std::shared_ptr<mtac::Param>>(&statement);
}

bool optimize_concat(std::shared_ptr<mtac::Function> function, std::shared_ptr<StringPool> pool){
    bool optimized = false;
    
    auto& blocks = function->getBasicBlocks();

    auto it = blocks.begin();
    auto end = blocks.end();
    auto previous = it;

    //we start at 1 because the first block cannot start with a call to concat
    ++it;

    while(it != end){
        auto block = *it;

        if(likely(!block->statements.empty())){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::Call>>(&block->statements[0])){
                if((*ptr)->function == "_F6concatSS"){
                    //The params are on the previous block
                    auto& paramBlock = *previous;

                    //Must have at least four params
                    if(paramBlock->statements.size() >= 4){
                        auto size = paramBlock->statements.size();
                        auto& statement1 = paramBlock->statements[size - 4];
                        auto& statement2 = paramBlock->statements[size - 3];
                        auto& statement3 = paramBlock->statements[size - 2];
                        auto& statement4 = paramBlock->statements[size - 1];

                        if(isParam(statement1) && isParam(statement2) && isParam(statement3) && isParam(statement4)){
                            auto& quadruple1 = boost::get<std::shared_ptr<mtac::Param>>(statement1);
                            auto& quadruple3 = boost::get<std::shared_ptr<mtac::Param>>(statement3);

                            if(boost::get<std::string>(&quadruple1->arg) && boost::get<std::string>(&quadruple3->arg)){
                                std::string firstValue = pool->value(boost::get<std::string>(quadruple1->arg));
                                std::string secondValue = pool->value(boost::get<std::string>(quadruple3->arg));

                                //Remove the quotes
                                firstValue.resize(firstValue.size() - 1);
                                secondValue.erase(0, 1);

                                //Compute the reuslt of the concatenation
                                std::string result = firstValue + secondValue;

                                std::string label = pool->label(result);
                                int length = result.length() - 2;

                                auto ret1 = (*ptr)->return_;
                                auto ret2 = (*ptr)->return2_;

                                //remove the call to concat
                                block->statements.erase(block->statements.begin());

                                //Insert assign with the concatenated value 
                                block->statements.insert(block->statements.begin(), std::make_shared<mtac::Quadruple>(ret1, label, mtac::Operator::ASSIGN));
                                block->statements.insert(block->statements.begin()+1, std::make_shared<mtac::Quadruple>(ret2, length, mtac::Operator::ASSIGN));

                                //Remove the four params from the previous basic block
                                paramBlock->statements.erase(paramBlock->statements.end() - 4, paramBlock->statements.end());

                                optimized = true;
                            }
                        }
                    }
                }
            }
        }

        previous = it;
        ++it;
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

bool remove_empty_functions(std::shared_ptr<mtac::Program> program){
    bool optimized = false;

    std::vector<std::string> removed_functions;

   auto it = program->functions.begin();
    
    while(it != program->functions.end()){
        auto function = *it;

        unsigned int statements = 0;

        for(auto& block : function->getBasicBlocks()){
            statements += block->statements.size();
        }

        if(statements == 0){
            removed_functions.push_back(function->getName());
            it = program->functions.erase(it);
        } else {
            ++it;
        }
    }

    if(!removed_functions.empty()){
        for(auto& function : program->functions){
            auto& blocks = function->getBasicBlocks();

            auto bit = blocks.begin();

            while(bit != blocks.end()){
                auto block = *bit;

                auto fit = block->statements.begin();

                while(fit != block->statements.end()){
                    auto statement = *fit;
                    
                    if(auto* ptr = boost::get<std::shared_ptr<mtac::Call>>(&statement)){
                        auto function = (*ptr)->function;

                        if(std::find(removed_functions.begin(), removed_functions.end(), function) != removed_functions.end()){
                            int parameters = (*ptr)->functionDefinition->parameters.size();

                            if(parameters > 0){
                                //The parameters are in the previous block
                                if(fit == block->statements.begin()){
                                    auto pit = bit;
                                    --pit;
                                    auto previous = *pit;

                                    auto fend = previous->statements.end();
                                    --fend;

                                    while(parameters > 0){
                                        fend = previous->statements.erase(fend);
                                        --fend;

                                        --parameters;
                                    }
                                   
                                    fit = block->statements.erase(fit);
                                } 
                                //The parameters are in the same block
                                else {
                                    while(parameters >= 0){
                                        fit = block->statements.erase(fit);
                                        --fit;

                                        --parameters;
                                    }
                                }

                            } else {
                                fit = block->statements.erase(fit);
                            }
                            
                            continue;
                        }
                    }

                    ++fit;
                }

                ++bit;
            }
        }
    }

    return optimized;
}

bool merge_basic_blocks(std::shared_ptr<mtac::Function> function){
    bool optimized = false;

    std::unordered_set<std::shared_ptr<mtac::BasicBlock>> usage;

    computeBlockUsage(function, usage);

    auto& blocks = function->getBasicBlocks();

    auto it = blocks.begin();

    //The ENTRY Basic block should not been merged
    ++it;

    while(it != blocks.end()){
        auto& block = *it;
        if(likely(!block->statements.empty())){
            auto& last = block->statements[block->statements.size() - 1];

            bool merge = false;

            if(boost::get<std::shared_ptr<mtac::Quadruple>>(&last)){
                merge = true;
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::Call>>(&last)){
                merge = safe(*ptr); 
            } else if(boost::get<std::shared_ptr<mtac::NoOp>>(&last)){
                merge = true;
            }

            auto next = it;
            ++next;

            if(merge && next != blocks.end() && (*next)->index != -2){
                //Only if the next block is not used because we will remove its label
                if(usage.find(*next) == usage.end()){
                    if(auto* ptr = boost::get<std::shared_ptr<mtac::Call>>(&(*(*next)->statements.begin()))){
                        if(!safe(*ptr)){
                            ++it;
                            continue;
                        }
                    }

                    block->statements.insert(block->statements.end(), (*next)->statements.begin(), (*next)->statements.end());

                    it = blocks.erase(next);
                    optimized = true;

                    --it;
                    continue;
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

bool dead_code_elimination(std::shared_ptr<mtac::Function> function){
    bool optimized = false;

    mtac::LiveVariableAnalysisProblem problem;

    auto results = mtac::data_flow(function, problem);

    for(auto& block : function->getBasicBlocks()){
        auto it = block->statements.begin();
        auto end = block->statements.end();

        while(it != end){
            auto statement = *it;

            if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
                if(mtac::erase_result((*ptr)->op)){
                    if(results->OUT_S[statement].values().find((*ptr)->result) == results->OUT_S[statement].values().end()){
                        it = block->statements.erase(it);
                        end = block->statements.end();
                        optimized=true;
                        continue;
                    }
                }
            }

            ++it;
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
        
        optimized |= debug("Dead-Code Elimination", dead_code_elimination(function), function);

        optimized |= debug("Optimize Branches", optimize_branches(function), function);
        optimized |= debug("Optimize Concat", optimize_concat(function, pool), function);
        optimized |= debug("Remove dead basic block", remove_dead_basic_blocks(function), function);
        optimized |= debug("Remove needless jumps", remove_needless_jumps(function), function);
    } while (optimized);
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

        optimized = remove_empty_functions(program);
    } while(optimized);
}

void mtac::Optimizer::basic_optimize(std::shared_ptr<mtac::Program> program, std::shared_ptr<StringPool> /*string_pool*/) const {
    PerfsTimer timer("Whole basic optimizations");

    auto& functions = program->functions;

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
