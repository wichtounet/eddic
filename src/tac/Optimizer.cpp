//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <boost/variant.hpp>
#include <boost/utility/enable_if.hpp>

#include "tac/Optimizer.hpp"
#include "tac/Program.hpp"
#include "tac/Utils.hpp"
#include "tac/OptimizerUtils.hpp"

//The optimization visitors
#include "tac/ArithmeticIdentities.hpp"
#include "tac/ReduceInStrength.hpp"
#include "tac/ConstantFolding.hpp"
#include "tac/ConstantPropagation.hpp"
#include "tac/CopyPropagation.hpp"
#include "tac/RemoveAssign.hpp"
#include "tac/RemoveMultipleAssign.hpp"

#include "VisitorUtils.hpp"
#include "StringPool.hpp"
#include "DebugStopWatch.hpp"

using namespace eddic;

namespace {

static const bool DebugPerf = false;
static const bool Debug = false;

struct MathPropagation : public boost::static_visitor<void> {
    bool optimized;
    tac::Pass pass;

    MathPropagation() : optimized(false) {}

    std::unordered_map<std::shared_ptr<Variable>, std::shared_ptr<tac::Quadruple>> assigns;
    std::unordered_map<std::shared_ptr<Variable>, int> usage;
    
    inline void collect(tac::Argument* arg){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(arg)){
            usage[*ptr] += 1;
        }
    }

    inline void collect(boost::optional<tac::Argument>& arg){
        if(arg){
            collect(&*arg);
        }
    }

    void operator()(std::shared_ptr<tac::Quadruple>& quadruple){
        if(pass == tac::Pass::DATA_MINING){
            collect(quadruple->arg1);
            collect(quadruple->arg2);
        } else {
            if(quadruple->result){
                assigns[quadruple->result] = quadruple;
            }

            if(quadruple->op == tac::Operator::ASSIGN){
                if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
                    //We only duplicate the math operation if the variable is used once to not add overhead
                    if(usage[*ptr] == 1 && assigns.find(*ptr) != assigns.end()){
                        auto assign = assigns[*ptr];
                        quadruple->op = assign->op;
                        quadruple->arg1 = assign->arg1;
                        quadruple->arg2 = assign->arg2;

                        optimized = true;
                    }
                }
            }
        }
    }

    template<typename T>
    inline void collectUsageFromBranch(T& if_){
        if(pass == tac::Pass::DATA_MINING){
            collect(&if_->arg1);
            collect(if_->arg2);
        }
    }
    
    void operator()(std::shared_ptr<tac::IfFalse>& ifFalse){
        collectUsageFromBranch(ifFalse);
    }
    
    void operator()(std::shared_ptr<tac::If>& if_){
        collectUsageFromBranch(if_);
    }
    
    template<typename T>
    void operator()(T&){ 
        //Do nothing for the others
    }
};

template<typename Visitor>
bool apply_to_all_clean(tac::Program& program){
    DebugStopWatch<DebugPerf> timer("apply to all clean");

    Visitor visitor;

    for(auto& function : program.functions){
        for(auto& block : function->getBasicBlocks()){
            visit_each(visitor, block->statements);
        }
    }

    return visitor.optimized;
}

template<typename Visitor>
bool apply_to_basic_blocks_clean(tac::Program& program){
    DebugStopWatch<DebugPerf> timer("apply to basic blocks");
    bool optimized = false;

    for(auto& function : program.functions){
        for(auto& block : function->getBasicBlocks()){
            Visitor visitor;

            visit_each(visitor, block->statements);

            optimized |= visitor.optimized;
        }
    }

    return optimized;
}

template<typename Visitor>
typename boost::disable_if<boost::is_void<typename Visitor::result_type>, bool>::type 
apply_to_basic_blocks_two_pass(tac::Program& program){
    DebugStopWatch<DebugPerf> timer("apply to basic blocks two phase");
    bool optimized = false;

    for(auto& function : program.functions){
        for(auto& block : function->getBasicBlocks()){
            Visitor visitor;
            visitor.pass = tac::Pass::DATA_MINING;

            //In the first pass, don't care about the return value
            visit_each(visitor, block->statements);

            visitor.pass = tac::Pass::OPTIMIZE;

            auto it = block->statements.begin();
            auto end = block->statements.end();
            
            block->statements.erase(
                std::remove_if(it, end,
                    [&](tac::Statement& s){return !visit(visitor, s); }), 
                end);

            optimized |= visitor.optimized;
        }
    }

    return optimized;
}

template<typename Visitor>
inline typename boost::enable_if<boost::is_void<typename Visitor::result_type>, bool>::type
apply_to_basic_blocks_two_pass(tac::Program& program){
    DebugStopWatch<DebugPerf> timer("apply to basic blocks two phase");
    bool optimized = false;

    for(auto& function : program.functions){
        for(auto& block : function->getBasicBlocks()){
            Visitor visitor;
            visitor.pass = tac::Pass::DATA_MINING;

            visit_each(visitor, block->statements);

            visitor.pass = tac::Pass::OPTIMIZE;

            visit_each(visitor, block->statements);

            optimized |= visitor.optimized;
        }
    }

    return optimized;
}

template<typename T>
unsigned int index(const std::vector<T>& vector, T& search){
    for(unsigned int i = 0; i < vector.size(); ++i){
        if(vector[i] == search){
            return i;
        }
    }

    assert(false);
}

bool remove_dead_basic_blocks(tac::Program& program){
    DebugStopWatch<DebugPerf> timer("Remove dead basic blocks");
    bool optimized = false;

    for(auto& function : program.functions){
        std::unordered_set<std::shared_ptr<tac::BasicBlock>> usage;

        auto& blocks = function->getBasicBlocks();

        unsigned int before = blocks.size();

        for(unsigned int i = 0; i < blocks.size();){
            usage.insert(blocks[i]);
            
            auto& block = blocks[i];
            if(block->statements.size() > 0){
                auto& last = block->statements[block->statements.size() - 1];

                if(auto* ptr = boost::get<std::shared_ptr<tac::Goto>>(&last)){
                    if(usage.find((*ptr)->block) == usage.end()){
                        i = index(blocks, (*ptr)->block);
                        continue;
                    }
                } else if(auto* ptr = boost::get<std::shared_ptr<tac::IfFalse>>(&last)){
                    usage.insert((*ptr)->block); 
                } else if(auto* ptr = boost::get<std::shared_ptr<tac::If>>(&last)){
                    usage.insert((*ptr)->block); 
                }
            }

            ++i;
        }

        auto it = blocks.begin();
        auto end = blocks.end();

        blocks.erase(
            std::remove_if(it, end, 
                [&](std::shared_ptr<tac::BasicBlock>& b){ return usage.find(b) == usage.end(); }), 
            end);

        if(blocks.size() < before){
            optimized = true;
        }
    }

    return optimized;
}

bool optimize_branches(tac::Program& program){
    DebugStopWatch<DebugPerf> timer("Optimize branches");
    bool optimized = false;
    
    for(auto& function : program.functions){
        for(auto& block : function->getBasicBlocks()){
            for(auto& statement : block->statements){
                if(auto* ptr = boost::get<std::shared_ptr<tac::IfFalse>>(&statement)){
                    if(!(*ptr)->op && boost::get<int>(&(*ptr)->arg1)){
                        int value = boost::get<int>((*ptr)->arg1);
                        
                        if(value == 0){
                            auto goto_ = std::make_shared<tac::Goto>();

                            goto_->label = (*ptr)->label;
                            goto_->block = (*ptr)->block;

                            statement = goto_;
                            optimized = true;
                        } else if(value == 1){
                            statement = tac::NoOp();
                            optimized = true;
                        }
                    }
                } else if(auto* ptr = boost::get<std::shared_ptr<tac::If>>(&statement)){
                    if(!(*ptr)->op && boost::get<int>(&(*ptr)->arg1)){
                        int value = boost::get<int>((*ptr)->arg1);
                        
                        if(value == 0){
                            statement = tac::NoOp();
                            optimized = true;
                        } else if(value == 1){
                            auto goto_ = std::make_shared<tac::Goto>();

                            goto_->label = (*ptr)->label;
                            goto_->block = (*ptr)->block;

                            statement = goto_;
                            optimized = true;
                        }
                    }
                }
            }
        }
    }

    return optimized;
}

template<typename T>
bool isQuadruple(T& statement){
    return boost::get<std::shared_ptr<tac::Quadruple>>(&statement);
}

bool isParam(std::shared_ptr<tac::Quadruple> quadruple){
    return quadruple->op == tac::Operator::PARAM;
}

bool optimize_concat(tac::Program& program, StringPool& pool){
    DebugStopWatch<DebugPerf> timer("Optimize concat");
    bool optimized = false;
    
    for(auto& function : program.functions){
        auto& blocks = function->getBasicBlocks();
        
        //we start at 1 because the first block cannot start with a call to concat
        for(unsigned int i = 1; i < blocks.size(); ++i){
            auto& block = blocks[i];

            if(block->statements.size() > 0){
                if(auto* ptr = boost::get<std::shared_ptr<tac::Call>>(&block->statements[0])){
                    if((*ptr)->function == "concat"){
                        //The params are on the previous block
                        auto& paramBlock = blocks[i - 1]; 

                        //Must have at least four params
                        if(paramBlock->statements.size() >= 4){
                            auto size = paramBlock->statements.size();
                            auto& statement1 = paramBlock->statements[size - 4];
                            auto& statement2 = paramBlock->statements[size - 3];
                            auto& statement3 = paramBlock->statements[size - 2];
                            auto& statement4 = paramBlock->statements[size - 1];

                            if(isQuadruple(statement1) && isQuadruple(statement2) && isQuadruple(statement3) && isQuadruple(statement4)){
                                auto& quadruple1 = boost::get<std::shared_ptr<tac::Quadruple>>(statement1);
                                auto& quadruple2 = boost::get<std::shared_ptr<tac::Quadruple>>(statement2);
                                auto& quadruple3 = boost::get<std::shared_ptr<tac::Quadruple>>(statement3);
                                auto& quadruple4 = boost::get<std::shared_ptr<tac::Quadruple>>(statement4);

                                if(isParam(quadruple1) && isParam(quadruple2) && isParam(quadruple3) && isParam(quadruple4)){
                                    if(boost::get<std::string>(&*quadruple1->arg1) && boost::get<std::string>(&*quadruple3->arg1)){
                                        std::string firstValue = pool.value(boost::get<std::string>(*quadruple1->arg1));
                                        std::string secondValue = pool.value(boost::get<std::string>(*quadruple3->arg1));
                                       
                                        //Remove the quotes
                                        firstValue.resize(firstValue.size() - 1);
                                        secondValue.erase(0, 1);
                                        
                                        //Compute the reuslt of the concatenation
                                        std::string result = firstValue + secondValue;

                                        std::string label = pool.label(result);
                                        int length = result.length() - 2;

                                        auto ret1 = (*ptr)->return_;
                                        auto ret2 = (*ptr)->return2_;

                                        //remove the call to concat
                                        block->statements.erase(block->statements.begin());
                                       
                                        //Insert assign with the concatenated value 
                                        block->statements.insert(block->statements.begin(), std::make_shared<tac::Quadruple>(ret1, label, tac::Operator::ASSIGN));
                                        block->statements.insert(block->statements.begin()+1, std::make_shared<tac::Quadruple>(ret2, length, tac::Operator::ASSIGN));

                                        //Remove the four params from the previous basic block
                                        paramBlock->statements.erase(paramBlock->statements.end() - 4, paramBlock->statements.end());

                                        optimized = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return optimized;
}

bool remove_needless_jumps(tac::Program& program){
    DebugStopWatch<DebugPerf> timer("Remove needless jumps");
    bool optimized = false;

    for(auto& function : program.functions){
        auto& blocks = function->getBasicBlocks();

        for(unsigned int i = 0; i < blocks.size();++i){
            auto& block = blocks[i];
            if(block->statements.size() > 0){
                auto& last = block->statements[block->statements.size() - 1];

                if(auto* ptr = boost::get<std::shared_ptr<tac::Goto>>(&last)){
                    unsigned int target = index(blocks, (*ptr)->block);
                   
                    if(target == i + 1){
                        block->statements.pop_back();
                        //erase(block->statements.size() - 1);

                        optimized = true;
                    }
                }
            }
        }
    }

    return optimized;
}

bool merge_basic_blocks(tac::Program& program){
    DebugStopWatch<DebugPerf> timer("Merge basic blocks");
    bool optimized = false;

    for(auto& function : program.functions){
        std::unordered_set<std::shared_ptr<tac::BasicBlock>> usage;
        
        computeBlockUsage(function, usage);

        auto& blocks = function->getBasicBlocks();

        auto it = blocks.begin();

        while(it != blocks.end()){
            auto& block = *it;
            if(block->statements.size() > 0){
                auto& last = block->statements[block->statements.size() - 1];

                bool merge = false;

                if(boost::get<std::shared_ptr<tac::Quadruple>>(&last)){
                    merge = true;
                } else if(auto* ptr = boost::get<std::shared_ptr<tac::Call>>(&last)){
                    merge = safe(*ptr); 
                } else if(boost::get<tac::NoOp>(&last)){
                    merge = true;
                }

                auto next = it + 1;
                if(merge && next != blocks.end()){
                    //Only if the next block is not used because we will remove its label
                    if(usage.find(*next) == usage.end()){
                        if(auto* ptr = boost::get<std::shared_ptr<tac::Call>>(&(*(*next)->statements.begin()))){
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
    }
   
    return optimized; 
}

template<bool Enabled, int i>
bool debug(bool b){
    if(Enabled){
        if(b){
            std::cout << "optimization " << i << " returned true" << std::endl;
        } else {
            std::cout << "optimization " << i << " returned false" << std::endl;
        }
    }

    return b;
}

}

void tac::Optimizer::optimize(tac::Program& program, StringPool& pool) const {
    bool optimized;
    do {
        optimized = false;

        //Optimize using arithmetic identities
        optimized |= debug<Debug, 1>(apply_to_all_clean<ArithmeticIdentities>(program));
        
        //Reduce arithtmetic instructions in strength
        optimized |= debug<Debug, 2>(apply_to_all_clean<ReduceInStrength>(program));

        //Constant folding
        optimized |= debug<Debug, 3>(apply_to_all_clean<ConstantFolding>(program));

        //Constant propagation
        optimized |= debug<Debug, 4>(apply_to_basic_blocks_clean<ConstantPropagation>(program));

        //Offset Constant propagation
        optimized |= debug<Debug, 5>(apply_to_basic_blocks_clean<OffsetConstantPropagation>(program));
        
        //Copy propagation
        optimized |= debug<Debug, 6>(apply_to_basic_blocks_clean<CopyPropagation>(program));
        
        //Offset Copy propagation
        optimized |= debug<Debug, 7>(apply_to_basic_blocks_clean<OffsetCopyPropagation>(program));

        //Propagate math
        optimized |= debug<Debug, 8>(apply_to_basic_blocks_two_pass<MathPropagation>(program));

        //Remove unused assignations
        optimized |= debug<Debug, 9>(apply_to_basic_blocks_two_pass<RemoveAssign>(program));

        //Remove unused assignations
        optimized |= debug<Debug, 10>(apply_to_basic_blocks_two_pass<RemoveMultipleAssign>(program));
       
        //Optimize branches 
        optimized |= debug<Debug, 11>(optimize_branches(program));
       
        //Optimize concatenations 
        optimized |= debug<Debug, 12>(optimize_concat(program, pool));

        //Remove dead basic blocks (unreachable code)
        optimized |= debug<Debug, 13>(remove_dead_basic_blocks(program));

        //Remove needless jumps
        optimized |= debug<Debug, 14>(remove_needless_jumps(program));

        //Merge basic blocks
        optimized |= debug<Debug, 15>(merge_basic_blocks(program));
    } while (optimized);
}
