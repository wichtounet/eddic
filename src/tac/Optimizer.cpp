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

#include "VisitorUtils.hpp"

#include "tac/Optimizer.hpp"
#include "tac/Program.hpp"
#include "tac/Utils.hpp"

using namespace eddic;

namespace {

static const bool Debug = false;

//Use for two pass optimization
enum class Pass : unsigned int {
    DATA_MINING,
    OPTIMIZE
};

struct ArithmeticIdentities : public boost::static_visitor<tac::Statement> {
    bool optimized;

    ArithmeticIdentities() : optimized(false) {}

    tac::Statement operator()(std::shared_ptr<tac::Quadruple>& quadruple){
        bool old = optimized;
        optimized = true;

        if(quadruple->op){
            switch(*quadruple->op){
                case tac::Operator::ADD:
                    if(tac::equals<int>(*quadruple->arg1, 0)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, *quadruple->arg2);
                    } else if(tac::equals<int>(*quadruple->arg2, 0)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, *quadruple->arg1);
                    }

                    break;
                case tac::Operator::SUB:
                    if(tac::equals<int>(*quadruple->arg2, 0)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, *quadruple->arg1);
                    } 

                    //a = b - b => a = 0
                    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
                        if(tac::equals<std::shared_ptr<Variable>>(*quadruple->arg2, *ptr)){
                            return std::make_shared<tac::Quadruple>(quadruple->result, 0);
                        }
                    }
                    
                    //a = 0 - b => a = -b
                    if(tac::equals<int>(*quadruple->arg1, 0)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, *quadruple->arg2, tac::Operator::MINUS);
                    }

                    break;
                case tac::Operator::MUL:
                    if(tac::equals<int>(*quadruple->arg1, 1)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, *quadruple->arg2);
                    } else if(tac::equals<int>(*quadruple->arg2, 1)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, *quadruple->arg1);
                    }
                    
                    if(tac::equals<int>(*quadruple->arg1, 0)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, 0);
                    } else if(tac::equals<int>(*quadruple->arg2, 0)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, 0);
                    }
                    
                    if(tac::equals<int>(*quadruple->arg1, -1)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, *quadruple->arg2, tac::Operator::MINUS);
                    } else if(tac::equals<int>(*quadruple->arg2, -1)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, *quadruple->arg1, tac::Operator::MINUS);
                    }

                    break;
                case tac::Operator::DIV:
                    if(tac::equals<int>(*quadruple->arg2, 1)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, *quadruple->arg1);
                    }

                    if(tac::equals<int>(*quadruple->arg1, 0)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, 0);
                    }

                    //a = b / b => a = 1
                    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
                        if(tac::equals<std::shared_ptr<Variable>>(*quadruple->arg2, *ptr)){
                            return std::make_shared<tac::Quadruple>(quadruple->result, 1);
                        }
                    }
                    
                    if(tac::equals<int>(*quadruple->arg2, 1)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, *quadruple->arg1, tac::Operator::MINUS);
                    }

                    break;
                default:
                    break;
            }
        }

        optimized = old;
        return quadruple;
    }

    template<typename T>
    tac::Statement operator()(T& statement) const { 
        return statement;
    }
};

struct ReduceInStrength : public boost::static_visitor<tac::Statement> {
    bool optimized;

    ReduceInStrength() : optimized(false) {}

    tac::Statement operator()(std::shared_ptr<tac::Quadruple>& quadruple){
        bool old = optimized;
        optimized = true;

        if(quadruple->op){
            switch(*quadruple->op){
                case tac::Operator::MUL:
                    if(tac::equals<int>(*quadruple->arg1, 2)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, *quadruple->arg2, tac::Operator::ADD, *quadruple->arg2);
                    } else if(tac::equals<int>(*quadruple->arg2, 2)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, *quadruple->arg1, tac::Operator::ADD, *quadruple->arg1);
                    }

                    break;
                default:
                    break;
            }
        }

        optimized = old;
        return quadruple;
    }

    template<typename T>
    tac::Statement operator()(T& statement) const { 
        return statement;
    }
};

struct ConstantFolding : public boost::static_visitor<tac::Statement> {
    bool optimized;

    ConstantFolding() : optimized(false) {}

    tac::Statement operator()(std::shared_ptr<tac::Quadruple>& quadruple){
        bool old = optimized;
        optimized = true;
        
        if(quadruple->op){
            switch(*quadruple->op){
                case tac::Operator::ADD:
                    if(tac::isInt(*quadruple->arg1) && tac::isInt(*quadruple->arg2)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, 
                            boost::get<int>(*quadruple->arg1) + boost::get<int>(*quadruple->arg2));
                    }

                    break;
                case tac::Operator::SUB:
                    if(tac::isInt(*quadruple->arg1) && tac::isInt(*quadruple->arg2)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, 
                            boost::get<int>(*quadruple->arg1) - boost::get<int>(*quadruple->arg2));
                    }

                    break;
                case tac::Operator::MUL:
                    if(tac::isInt(*quadruple->arg1) && tac::isInt(*quadruple->arg2)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, 
                            boost::get<int>(*quadruple->arg1) * boost::get<int>(*quadruple->arg2));
                    }

                    break;
                case tac::Operator::DIV:
                    if(tac::isInt(*quadruple->arg1) && tac::isInt(*quadruple->arg2)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, 
                            boost::get<int>(*quadruple->arg1) / boost::get<int>(*quadruple->arg2));
                    }

                    break;
                case tac::Operator::MOD:
                    if(tac::isInt(*quadruple->arg1) && tac::isInt(*quadruple->arg2)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, 
                            boost::get<int>(*quadruple->arg1) % boost::get<int>(*quadruple->arg2));
                    }
                case tac::Operator::MINUS:
                    if(tac::isInt(*quadruple->arg1)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, -1 * boost::get<int>(*quadruple->arg1));
                    }

                    break;
                default:
                    break;
            }
        }

        optimized = old;
        return quadruple;
    }

    template<typename T>
    bool computeValue(T& if_){
        int left = boost::get<int>(if_->arg1);
        int right = boost::get<int>(*if_->arg2);

        bool value = false;

        switch(*if_->op){
            case tac::BinaryOperator::EQUALS:
                value = left == right;

                break;
            case tac::BinaryOperator::NOT_EQUALS:
                value = left != right;

                break;
            case tac::BinaryOperator::LESS:
                value = left < right;

                break;
            case tac::BinaryOperator::LESS_EQUALS:
                value = left <= right;

                break;
            case tac::BinaryOperator::GREATER:
                value = left > right;

                break;
            case tac::BinaryOperator::GREATER_EQUALS:
                value = left >= right;

                break;
        }
       
        return value; 
    }

    tac::Statement operator()(std::shared_ptr<tac::IfFalse>& ifFalse){
        if(ifFalse->op){
            if(tac::isInt(ifFalse->arg1) && tac::isInt(*ifFalse->arg2)){
                bool value = computeValue(ifFalse);

                //TODO Do the replacing by NoOp or Goto in another pass of optimization, only constant folding there

                //replace if_false true by no-op
                if(value){
                    return tac::NoOp();
                } 
                //replace if_false false by goto 
                else {
                    auto goto_ = std::make_shared<tac::Goto>();

                    goto_->label = ifFalse->label;
                    goto_->block = ifFalse->block;

                    return goto_; 
                }
            }
        }

        return ifFalse;
    }

    tac::Statement operator()(std::shared_ptr<tac::If>& if_){
        if(if_->op){
            if(tac::isInt(if_->arg1) && tac::isInt(*if_->arg2)){
                bool value = computeValue(if_);

                //TODO Do the replacing by NoOp or Goto in another pass of optimization, only constant folding there

                //replace if true by goto
                if(value){
                    auto goto_ = std::make_shared<tac::Goto>();

                    goto_->label = if_->label;
                    goto_->block = if_->block;

                    return goto_; 
                }
                //replace if false by no-op 
                else {
                    return tac::NoOp();
                }
            }
        }

        return if_;
    }

    template<typename T>
    tac::Statement operator()(T& statement) const { 
        return statement;
    }
};

struct ConstantPropagation : public boost::static_visitor<tac::Statement> {
    bool optimized;

    ConstantPropagation() : optimized(false) {}

    std::unordered_map<std::shared_ptr<Variable>, int> constants;

    tac::Statement operator()(std::shared_ptr<tac::Quadruple>& quadruple){
        if(!quadruple->op){
            if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
                if(constants.find(*ptr) != constants.end()){
                    optimized = true;
                    quadruple->arg1 = constants[*ptr];
                }
            }

            if(auto* ptr = boost::get<int>(&*quadruple->arg1)){
                constants[quadruple->result] = *ptr;
            } else {
                //The result is not constant at this point
                constants.erase(quadruple->result);
            }
        } else {
            if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
                if(constants.find(*ptr) != constants.end()){
                    optimized = true;
                    quadruple->arg1 = constants[*ptr];
                }
            }

            if(quadruple->arg2){
                if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg2)){
                    if(constants.find(*ptr) != constants.end()){
                        optimized = true;
                        quadruple->arg2 = constants[*ptr];
                    }
                }
            }

            //The result is not constant at this point
            constants.erase(quadruple->result);
        }

        return quadruple;
    }

    template<typename T>
    tac::Statement optimizeBranch(T& if_){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&if_->arg1)){
            if(constants.find(*ptr) != constants.end()){
                optimized = true;
                if_->arg1 = constants[*ptr];
            }
        }
       
        if(if_->arg2){ 
            if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*if_->arg2)){
                if(constants.find(*ptr) != constants.end()){
                    optimized = true;
                    if_->arg2 = constants[*ptr];
                }
            }
        }

        return if_;
    }

    tac::Statement operator()(std::shared_ptr<tac::IfFalse>& ifFalse){
        return optimizeBranch(ifFalse);
    }
    
    tac::Statement operator()(std::shared_ptr<tac::If>& if_){
        return optimizeBranch(if_);
    }

    tac::Statement operator()(std::shared_ptr<tac::Return>& return_){
        if(return_->arg1){
            if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*return_->arg1)){
                if(constants.find(*ptr) != constants.end()){
                    optimized = true;
                    return_->arg1 = constants[*ptr];
                }
            }
        }

        if(return_->arg2){
            if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*return_->arg2)){
                if(constants.find(*ptr) != constants.end()){
                    optimized = true;
                    return_->arg2 = constants[*ptr];
                }
            }
        }

        return return_;
    }

    template<typename T>
    tac::Statement operator()(T& statement){ 
        return statement;
    }
};

struct RemoveAssign : public boost::static_visitor<bool> {
    bool optimized;
    Pass pass;

    RemoveAssign() : optimized(false) {}

    std::unordered_set<std::shared_ptr<Variable>> used;

    bool operator()(std::shared_ptr<tac::Quadruple>& quadruple){
        if(pass == Pass::DATA_MINING){
            if(!quadruple->op){
                if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
                    used.insert(*ptr);
                }
            } else {
                if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
                    used.insert(*ptr);
                }

                if(quadruple->arg2){
                    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg2)){
                        used.insert(*ptr);
                    }
                }
            }
            
            return true;
        } else {
            //These operators are not erasing result
            if(quadruple->op && (*quadruple->op == tac::Operator::PARAM || *quadruple->op == tac::Operator::DOT_ASSIGN || *quadruple->op == tac::Operator::ARRAY_ASSIGN)){
                return true;
            }

            if(used.find(quadruple->result) == used.end()){
                //The other kind of variables can be used in other basic block
                if(quadruple->result->position().isTemporary()){
                    optimized = true;
                    return false;
                }
            }

            return true;
        }
    }

    template<typename T>
    bool collectUsageFromBranch(T& if_){
        if(pass == Pass::DATA_MINING){
            if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&if_->arg1)){
                used.insert(*ptr);
            }

            if(if_->arg2){
                if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*if_->arg2)){
                    used.insert(*ptr);
                }
            }
        }

        return true;
    }
    
    bool operator()(std::shared_ptr<tac::IfFalse>& ifFalse){
        return collectUsageFromBranch(ifFalse);
    }
    
    bool operator()(std::shared_ptr<tac::If>& if_){
        return collectUsageFromBranch(if_);
    }

    bool operator()(std::shared_ptr<tac::Return>& return_){
        if(pass == Pass::DATA_MINING){
            if(return_->arg1){
                if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*return_->arg1)){
                    used.insert(*ptr);
                }
            }

            if(return_->arg2){
                if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*return_->arg2)){
                    used.insert(*ptr);
                }
            }
        }

        return true;
    }
    
    template<typename T>
    bool operator()(T&){ 
        return true;
    }
};

template<typename Visitor>
bool apply_to_all(tac::Program& program){
    Visitor visitor;

    for(auto& function : program.functions){
        for(auto& block : function->getBasicBlocks()){
            for(auto& statement : block->statements){
                statement = visit(visitor, statement);
            }
        }
    }

    return visitor.optimized;
}

template<typename Visitor>
bool apply_to_basic_blocks(tac::Program& program){
    bool optimized = false;

    for(auto& function : program.functions){
        for(auto& block : function->getBasicBlocks()){
            Visitor visitor;

            for(auto& statement : block->statements){
                statement = visit(visitor, statement);
            }

            optimized |= visitor.optimized;
        }
    }

    return optimized;
}

template<typename Visitor>
bool apply_to_basic_blocks_two_pass(tac::Program& program){
    bool optimized = false;

    for(auto& function : program.functions){
        for(auto& block : function->getBasicBlocks()){
            Visitor visitor;
            visitor.pass = Pass::DATA_MINING;

            auto it = block->statements.begin();
            auto end = block->statements.end();

            while(it != end){
                bool keep = visit(visitor, *it);
                
                if(!keep){
                    it = block->statements.erase(it);   
                }

                it++;
            }

            visitor.pass = Pass::OPTIMIZE;

            it = block->statements.begin();
            end = block->statements.end();
            
            block->statements.erase(
                std::remove_if(it, end,
                    [&](tac::Statement& s){return !visit(visitor, s); }), 
                end);

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

bool remove_needless_jumps(tac::Program& program){
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
    bool optimized = false;

    std::unordered_set<std::shared_ptr<tac::BasicBlock>> usage;

    for(auto& function : program.functions){
        computeBlockUsage(function, usage);
    }
    
    for(auto& function : program.functions){
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

void tac::Optimizer::optimize(tac::Program& program) const {
    bool optimized;
    do {
        optimized = false;

        //Optimize using arithmetic identities
        optimized |= debug<Debug, 1>(apply_to_all<ArithmeticIdentities>(program));
        
        //Reduce arithtmetic instructions in strength
        optimized |= debug<Debug, 2>(apply_to_all<ReduceInStrength>(program));

        //Constant folding
        optimized |= debug<Debug, 3>(apply_to_all<ConstantFolding>(program));

        //Constant propagation
        optimized |= debug<Debug, 4>(apply_to_basic_blocks<ConstantPropagation>(program));

        //Remove unused assignations
        optimized |= debug<Debug, 5>(apply_to_basic_blocks_two_pass<RemoveAssign>(program));

        //Remove dead basic blocks (unreachable code)
        optimized |= debug<Debug, 6>(remove_dead_basic_blocks(program));

        //Remove needless jumps
        optimized |= debug<Debug, 7>(remove_needless_jumps(program));

        //Merge basic blocks
        optimized |= debug<Debug, 8>(merge_basic_blocks(program));
    } while (optimized);
    
    //TODO Copy propagation
}
