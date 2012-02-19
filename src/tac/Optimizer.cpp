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

#include "VisitorUtils.hpp"
#include "StringPool.hpp"
#include "DebugStopWatch.hpp"

using namespace eddic;

namespace {

static const bool Debug = true;

//Use for two pass optimization
enum class Pass : unsigned int {
    DATA_MINING,
    OPTIMIZE
};

template<typename T>
inline void replaceRight(T& visitor, std::shared_ptr<tac::Quadruple>& quadruple, tac::Argument arg){
    visitor.optimized = true;

    quadruple->op.reset();
    quadruple->arg1 = arg;
    quadruple->arg2.reset();
}

template<typename T>
inline void replaceRight(T& visitor, std::shared_ptr<tac::Quadruple>& quadruple, tac::Argument arg, tac::Operator op){
    visitor.optimized = true;

    quadruple->op = op;
    quadruple->arg1 = arg;
    quadruple->arg2.reset();
}

template<typename T>
inline void replaceRight(T& visitor, std::shared_ptr<tac::Quadruple>& quadruple, tac::Argument arg1, tac::Operator op, tac::Argument arg2){
    visitor.optimized = true;

    quadruple->op = op;
    quadruple->arg1 = arg1;
    quadruple->arg2 = arg2;
}

struct ArithmeticIdentities : public boost::static_visitor<void> {
    bool optimized;

    ArithmeticIdentities() : optimized(false) {}

    void operator()(std::shared_ptr<tac::Quadruple>& quadruple){
        if(quadruple->op){
            switch(*quadruple->op){
                case tac::Operator::ADD:
                    if(*quadruple->arg1 == 0){
                        replaceRight(*this, quadruple, *quadruple->arg2);
                    } else if(*quadruple->arg2 == 0){
                        replaceRight(*this, quadruple, *quadruple->arg1);
                    }

                    break;
                case tac::Operator::SUB:
                    if(*quadruple->arg2 == 0){
                        replaceRight(*this, quadruple, *quadruple->arg1);
                    } 

                    //a = b - b => a = 0
                    if(*quadruple->arg1 == *quadruple->arg2){
                        replaceRight(*this, quadruple, 0);
                    }
                    
                    //a = 0 - b => a = -b
                    if(*quadruple->arg1 == 0){
                        replaceRight(*this, quadruple, *quadruple->arg2, tac::Operator::MINUS);
                    }

                    break;
                case tac::Operator::MUL:
                    if(*quadruple->arg1 == 1){
                        replaceRight(*this, quadruple, *quadruple->arg2);
                    } else if(*quadruple->arg2 == 1){
                        replaceRight(*this, quadruple, *quadruple->arg1);
                    }
                    
                    if(*quadruple->arg1 == 0){
                        replaceRight(*this, quadruple, 0);
                    } else if(*quadruple->arg2 == 0){
                        replaceRight(*this, quadruple, 0);
                    }
                    
                    if(*quadruple->arg1 == -1){
                        replaceRight(*this, quadruple, *quadruple->arg2, tac::Operator::MINUS);
                    } else if(*quadruple->arg2 == -1){
                        replaceRight(*this, quadruple, *quadruple->arg1, tac::Operator::MINUS);
                    }

                    break;
                case tac::Operator::DIV:
                    if(*quadruple->arg2 == 1){
                        replaceRight(*this, quadruple, *quadruple->arg1);
                    }

                    if(*quadruple->arg1 == 0){
                        replaceRight(*this, quadruple, 0);
                    }

                    //a = b / b => a = 1
                    if(*quadruple->arg1 == *quadruple->arg2){
                        replaceRight(*this, quadruple, 1);
                    }
                    
                    if(*quadruple->arg2 == 1){
                        replaceRight(*this, quadruple, *quadruple->arg1, tac::Operator::MINUS);
                    }

                    break;
                default:
                    break;
            }
        }
    }

    template<typename T>
    void operator()(T&) const { 
        //Nothing to optimize
    }
};

struct ReduceInStrength : public boost::static_visitor<void> {
    bool optimized;

    ReduceInStrength() : optimized(false) {}

    void operator()(std::shared_ptr<tac::Quadruple>& quadruple){
        if(quadruple->op){
            switch(*quadruple->op){
                case tac::Operator::MUL:
                    if(*quadruple->arg1 == 2){
                        replaceRight(*this, quadruple, *quadruple->arg2, tac::Operator::ADD, *quadruple->arg2);
                    } else if(*quadruple->arg2 == 2){
                        replaceRight(*this, quadruple, *quadruple->arg1, tac::Operator::ADD, *quadruple->arg1);
                    }

                    break;
                default:
                    break;
            }
        }
    }

    template<typename T>
    void operator()(T&) const { 
        //Nothing to optimize
    }
};

struct ConstantFolding : public boost::static_visitor<void> {
    bool optimized;

    ConstantFolding() : optimized(false) {}

    void operator()(std::shared_ptr<tac::Quadruple>& quadruple){
        if(quadruple->op){
            if(quadruple->arg1 && tac::isInt(*quadruple->arg1)){
                if(*quadruple->op == tac::Operator::MINUS){
                    replaceRight(*this, quadruple, -1 * boost::get<int>(*quadruple->arg1));
                } else if(quadruple->arg2 && tac::isInt(*quadruple->arg2)){
                    int lhs = boost::get<int>(*quadruple->arg1); 
                    int rhs = boost::get<int>(*quadruple->arg2); 

                    switch(*quadruple->op){
                        case tac::Operator::ADD:
                            replaceRight(*this, quadruple, lhs + rhs);
                            break;
                        case tac::Operator::SUB:
                            replaceRight(*this, quadruple, lhs - rhs);
                            break;
                        case tac::Operator::MUL:
                            replaceRight(*this, quadruple, lhs * rhs);
                            break;
                        case tac::Operator::DIV:
                            replaceRight(*this, quadruple, lhs / rhs);
                            break;
                        case tac::Operator::MOD:
                            replaceRight(*this, quadruple, lhs % rhs);
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }

    template<typename T>
    bool computeValue(T& if_){
        int left = boost::get<int>(if_->arg1);
        int right = boost::get<int>(*if_->arg2);

        switch(*if_->op){
            case tac::BinaryOperator::EQUALS:
                return left == right;
            case tac::BinaryOperator::NOT_EQUALS:
                return left != right;
            case tac::BinaryOperator::LESS:
                return left < right;
            case tac::BinaryOperator::LESS_EQUALS:
                return left <= right;
            case tac::BinaryOperator::GREATER:
                return left > right;
            case tac::BinaryOperator::GREATER_EQUALS:
                return left >= right;
            default:
                assert(false);
        }
    }

    void operator()(std::shared_ptr<tac::IfFalse>& ifFalse){
        if(ifFalse->op){
            if(tac::isInt(ifFalse->arg1) && tac::isInt(*ifFalse->arg2)){
                bool value = computeValue(ifFalse);

                ifFalse->op.reset();
                ifFalse->arg1 = value ? 1 : 0;
                ifFalse->arg2.reset();

/*

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
                */
            }
        }
    }

    void operator()(std::shared_ptr<tac::If>& if_){
        if(if_->op){
            if(tac::isInt(if_->arg1) && tac::isInt(*if_->arg2)){
                bool value = computeValue(if_);

                if_->op.reset();
                if_->arg1 = value ? 1 : 0;
                if_->arg2.reset();

/*
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
                }*/
            }
        }
    }

    template<typename T>
    void operator()(T&) const { 
        //Nothing to optimize
    }
};

struct ConstantPropagation : public boost::static_visitor<tac::Statement> {
    bool optimized;

    ConstantPropagation() : optimized(false) {}

    std::unordered_map<std::shared_ptr<Variable>, int> int_constants;
    std::unordered_map<std::shared_ptr<Variable>, std::string> string_constants;

    void optimize(tac::Argument* arg){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(arg)){
            if(int_constants.find(*ptr) != int_constants.end()){
                optimized = true;
                *arg = int_constants[*ptr];
            } else if(string_constants.find(*ptr) != string_constants.end()){
                optimized = true;
                *arg = string_constants[*ptr];
            }
        }
    }

    void optimize_optional(boost::optional<tac::Argument>& arg){
        if(arg){
            optimize(&*arg);
        }
    }

    tac::Statement operator()(std::shared_ptr<tac::Quadruple>& quadruple){
        //Do not replace a variable by a constant when used in offset
        if(!quadruple->op || (*quadruple->op != tac::Operator::ARRAY && *quadruple->op != tac::Operator::DOT)){
            optimize_optional(quadruple->arg1);
        }
        
        optimize_optional(quadruple->arg2);

        if(!quadruple->op){
            if(auto* ptr = boost::get<int>(&*quadruple->arg1)){
                int_constants[quadruple->result] = *ptr;
            } else if(auto* ptr = boost::get<std::string>(&*quadruple->arg1)){
                string_constants[quadruple->result] = *ptr;
            } else {
                //The result is not constant at this point
                int_constants.erase(quadruple->result);
                string_constants.erase(quadruple->result);
            }
        } else {
            auto op = *quadruple->op;

            //Check if the operator erase the contents of the result variable
            if(op != tac::Operator::ARRAY_ASSIGN && op != tac::Operator::DOT_ASSIGN && op != tac::Operator::PARAM && op != tac::Operator::RETURN){
                //The result is not constant at this point
                int_constants.erase(quadruple->result);
                string_constants.erase(quadruple->result);
            }
        }

        return quadruple;
    }

    template<typename T>
    tac::Statement optimizeBranch(T& if_){
        optimize(&if_->arg1);
        optimize_optional(if_->arg2);

        return if_;
    }

    tac::Statement operator()(std::shared_ptr<tac::IfFalse>& ifFalse){
        return optimizeBranch(ifFalse);
    }
    
    tac::Statement operator()(std::shared_ptr<tac::If>& if_){
        return optimizeBranch(if_);
    }

    template<typename T>
    tac::Statement operator()(T& statement){ 
        return statement;
    }
};

struct Offset {
    std::shared_ptr<Variable> variable;
    int offset;

    bool operator==(const Offset& b) const {
        return variable == b.variable && offset == b.offset;
    }
};

template <class T>
inline void hash_combine(std::size_t& seed, const T& v){
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

struct OffsetHash : std::unary_function<Offset, std::size_t> {
    std::size_t operator()(const Offset& p) const {
        std::size_t seed = 0;
        hash_combine(seed, p.variable);
        hash_combine(seed, p.offset);
        return seed;
    }
};

struct OffsetConstantPropagation : public boost::static_visitor<tac::Statement> {
    bool optimized;

    OffsetConstantPropagation() : optimized(false) {}

    std::unordered_map<Offset, int, OffsetHash> int_constants;
    std::unordered_map<Offset, std::string, OffsetHash> string_constants;

    tac::Statement operator()(std::shared_ptr<tac::Quadruple>& quadruple){
        //Store the value assigned to result+arg1
        if(quadruple->op && *quadruple->op == tac::Operator::DOT_ASSIGN){
            if(auto* ptr = boost::get<int>(&*quadruple->arg1)){
                Offset offset;
                offset.variable = quadruple->result;
                offset.offset = *ptr;
                
                if(auto* ptr = boost::get<int>(&*quadruple->arg2)){
                    int_constants[offset] = *ptr;
                } else if(auto* ptr = boost::get<std::string>(&*quadruple->arg2)){
                    string_constants[offset] = *ptr;
                } else {
                    //The result is not constant at this point
                    int_constants.erase(offset);
                    string_constants.erase(offset);
                }
            }
        }
        
        //If constant replace the value assigned to result by the value stored for arg1+arg2
        if(quadruple->op && *quadruple->op == tac::Operator::DOT){
            if(auto* ptr = boost::get<int>(&*quadruple->arg2)){
                Offset offset;
                offset.variable = boost::get<std::shared_ptr<Variable>>(*quadruple->arg1);
                offset.offset = *ptr;
               
                if(int_constants.find(offset) != int_constants.end()){
                    optimized = true;
                    return std::make_shared<tac::Quadruple>(quadruple->result, int_constants[offset]);
                } else if(string_constants.find(offset) != string_constants.end()){
                    optimized = true;
                    return std::make_shared<tac::Quadruple>(quadruple->result, string_constants[offset]); 
                }
            }
        }

        return quadruple;
    }

    template<typename T>
    tac::Statement operator()(T& statement){ 
        return statement;
    }
};

struct CopyPropagation : public boost::static_visitor<tac::Statement> {
    bool optimized;

    CopyPropagation() : optimized(false) {}

    std::unordered_map<std::shared_ptr<Variable>, std::shared_ptr<Variable>> constants;

    void optimize(tac::Argument* arg){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(arg)){
            if(constants.find(*ptr) != constants.end() && constants[*ptr] != *ptr){
                optimized = true;
                *arg = constants[*ptr];
            }
        }
    }

    void optimize_optional(boost::optional<tac::Argument>& arg){
        if(arg){
            optimize(&*arg);
        }
    }

    tac::Statement operator()(std::shared_ptr<tac::Quadruple>& quadruple){
        //Do not replace a variable by a constant when used in offset
        if(!quadruple->op || (*quadruple->op != tac::Operator::ARRAY && *quadruple->op != tac::Operator::DOT)){
            optimize_optional(quadruple->arg1);
        }

        //optimize_optional(quadruple->arg1);
        optimize_optional(quadruple->arg2);
        
        if(!quadruple->op){
            if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
                //if(!(*ptr)->position().isTemporary()){
                    constants[quadruple->result] = *ptr;
                //}
            } else {
                //The result is not constant at this point
                constants.erase(quadruple->result);
            }
        } else {
            auto op = *quadruple->op;

            //Check if the operator erase the contents of the result variable
            if(op != tac::Operator::ARRAY_ASSIGN && op != tac::Operator::DOT_ASSIGN && op != tac::Operator::PARAM && op != tac::Operator::RETURN){
                //The result is not constant at this point
                constants.erase(quadruple->result);
            }
        }

        return quadruple;
    }

    template<typename T>
    tac::Statement optimizeBranch(T& if_){
        optimize(&if_->arg1);
        optimize_optional(if_->arg2);

        return if_;
    }

    tac::Statement operator()(std::shared_ptr<tac::IfFalse>& ifFalse){
        return optimizeBranch(ifFalse);
    }
    
    tac::Statement operator()(std::shared_ptr<tac::If>& if_){
        return optimizeBranch(if_);
    }

    template<typename T>
    tac::Statement operator()(T& statement){ 
        return statement;
    }
};

struct OffsetCopyPropagation : public boost::static_visitor<tac::Statement> {
    bool optimized;

    OffsetCopyPropagation() : optimized(false) {}

    std::unordered_map<Offset, std::shared_ptr<Variable>, OffsetHash> constants;

    tac::Statement operator()(std::shared_ptr<tac::Quadruple>& quadruple){
        //Store the value assigned to result+arg1
        if(quadruple->op && *quadruple->op == tac::Operator::DOT_ASSIGN){
            if(auto* ptr = boost::get<int>(&*quadruple->arg1)){
                Offset offset;
                offset.variable = quadruple->result;
                offset.offset = *ptr;
                
                if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg2)){
                    constants[offset] = *ptr;
                } else {
                    //The result is not constant at this point
                    constants.erase(offset);
                }
            }
        }
        
        //If constant replace the value assigned to result by the value stored for arg1+arg2
        if(quadruple->op && *quadruple->op == tac::Operator::DOT){
            if(auto* ptr = boost::get<int>(&*quadruple->arg2)){
                Offset offset;
                offset.variable = boost::get<std::shared_ptr<Variable>>(*quadruple->arg1);
                offset.offset = *ptr;
               
                if(constants.find(offset) != constants.end()){
                    optimized = true;
                    return std::make_shared<tac::Quadruple>(quadruple->result, constants[offset]);
                }
            }
        }

        return quadruple;
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

    void collect(tac::Argument* arg){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(arg)){
            used.insert(*ptr);
        }
    }

    void collect_optional(boost::optional<tac::Argument>& arg){
        if(arg){
            collect(&*arg);
        }
    }

    bool operator()(std::shared_ptr<tac::Quadruple>& quadruple){
        if(pass == Pass::DATA_MINING){
            collect_optional(quadruple->arg1);
            collect_optional(quadruple->arg2);
            
            return true;
        } else {
            //These operators are not erasing result
            if(quadruple->op && (*quadruple->op == tac::Operator::PARAM || *quadruple->op == tac::Operator::DOT_ASSIGN || *quadruple->op == tac::Operator::ARRAY_ASSIGN)){
                return true;
            }

            //x = x is never useful
            if(!quadruple->op && *quadruple->arg1 == quadruple->result){
                optimized = true;
                return false;
            }

            if(used.find(quadruple->result) == used.end()){
                if(quadruple->result){
                    //The other kind of variables can be used in other basic block
                    if(quadruple->result->position().isTemporary()){
                        optimized = true;
                        return false;
                    }
                }
            }

            return true;
        }
    }

    template<typename T>
    bool collectUsageFromBranch(T& if_){
        if(pass == Pass::DATA_MINING){
            collect(&if_->arg1);
            collect_optional(if_->arg2);
        }

        return true;
    }
    
    bool operator()(std::shared_ptr<tac::IfFalse>& ifFalse){
        return collectUsageFromBranch(ifFalse);
    }
    
    bool operator()(std::shared_ptr<tac::If>& if_){
        return collectUsageFromBranch(if_);
    }
    
    template<typename T>
    bool operator()(T&){ 
        return true;
    }
};

struct RemoveMultipleAssign : public boost::static_visitor<bool> {
    bool optimized;
    Pass pass;

    RemoveMultipleAssign() : optimized(false) {}

    std::unordered_set<std::shared_ptr<Variable>> used;
    std::unordered_map<std::shared_ptr<Variable>, std::shared_ptr<tac::Quadruple>> lastAssign;
    std::unordered_set<std::shared_ptr<tac::Quadruple>> removed;

    void collect(tac::Argument* arg){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*arg)){
            used.insert(*ptr);
        }
    }

    void collect(boost::optional<tac::Argument>& arg){
        if(arg){
            collect(&*arg);
        }
    }

    bool operator()(std::shared_ptr<tac::Quadruple>& quadruple){
        if(pass == Pass::DATA_MINING){
            collect(quadruple->arg1);
            collect(quadruple->arg2);
            
            //These operators are not erasing result
            if(quadruple->op && (*quadruple->op == tac::Operator::PARAM || *quadruple->op == tac::Operator::DOT_ASSIGN || *quadruple->op == tac::Operator::ARRAY_ASSIGN)){
                return true;
            }
            
            if(quadruple->result){
                //If the variable have not been used since the last assign
                if(used.find(quadruple->result) == used.end() && lastAssign.find(quadruple->result) != lastAssign.end()){
                    //Mark the last assign as useless
                    removed.insert(lastAssign[quadruple->result]);

                    optimized = true;
                }

                used.erase(quadruple->result);
                lastAssign[quadruple->result] = quadruple;
            }
            
            return true;
        } else {
            if(removed.find(quadruple) != removed.end()){
                return false;
            }

            return true;
        }
    }

    template<typename T>
    bool collectUsageFromBranch(T& if_){
        if(pass == Pass::DATA_MINING){
            collect(&if_->arg1);
            collect(if_->arg2);
        }

        return true;
    }
    
    bool operator()(std::shared_ptr<tac::IfFalse>& ifFalse){
        return collectUsageFromBranch(ifFalse);
    }
    
    bool operator()(std::shared_ptr<tac::If>& if_){
        return collectUsageFromBranch(if_);
    }
    
    template<typename T>
    bool operator()(T&){ 
        return true;
    }
};

struct MathPropagation : public boost::static_visitor<void> {
    bool optimized;
    Pass pass;

    MathPropagation() : optimized(false) {}

    std::unordered_map<std::shared_ptr<Variable>, std::shared_ptr<tac::Quadruple>> assigns;
    std::unordered_map<std::shared_ptr<Variable>, int> usage;
    
    void collect(tac::Argument* arg){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(arg)){
            usage[*ptr] += 1;
        }
    }

    void collect(boost::optional<tac::Argument>& arg){
        if(arg){
            collect(&*arg);
        }
    }

    void operator()(std::shared_ptr<tac::Quadruple>& quadruple){
        if(pass == Pass::DATA_MINING){
            collect(quadruple->arg1);
            collect(quadruple->arg2);
        } else {
            if(quadruple->result){
                assigns[quadruple->result] = quadruple;
            }

            if(!quadruple->op){
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
    void collectUsageFromBranch(T& if_){
        if(pass == Pass::DATA_MINING){
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

//TODO Delete this function
template<typename Visitor>
bool apply_to_all(tac::Program& program){
    DebugStopWatch<Debug> timer("apply to all");

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
bool apply_to_all_clean(tac::Program& program){
    DebugStopWatch<Debug> timer("apply to all clean");

    Visitor visitor;

    for(auto& function : program.functions){
        for(auto& block : function->getBasicBlocks()){
            visit_each(visitor, block->statements);
        }
    }

    return visitor.optimized;
}

template<typename Visitor>
bool apply_to_basic_blocks(tac::Program& program){
    DebugStopWatch<Debug> timer("apply to basic blocks");
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
typename boost::disable_if<boost::is_void<typename Visitor::result_type>, bool>::type 
apply_to_basic_blocks_two_pass(tac::Program& program){
    DebugStopWatch<Debug> timer("apply to basic blocks two phase");
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

template<typename Visitor>
inline typename boost::enable_if<boost::is_void<typename Visitor::result_type>, bool>::type
apply_to_basic_blocks_two_pass(tac::Program& program){
    DebugStopWatch<Debug> timer("apply to basic blocks two phase");
    bool optimized = false;

    for(auto& function : program.functions){
        for(auto& block : function->getBasicBlocks()){
            Visitor visitor;
            visitor.pass = Pass::DATA_MINING;

            for(auto& statement : block->statements){
                visit(visitor, statement);
            }

            visitor.pass = Pass::OPTIMIZE;

            for(auto& statement : block->statements){
                visit(visitor, statement);
            }

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
    DebugStopWatch<Debug> timer("Remove dead basic blocks");
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

template<typename T>
bool isQuadruple(T& statement){
    return boost::get<std::shared_ptr<tac::Quadruple>>(&statement);
}

bool isParam(std::shared_ptr<tac::Quadruple> quadruple){
    return quadruple->op && *quadruple->op == tac::Operator::PARAM;
}

bool optimize_concat(tac::Program& program, StringPool& pool){
    DebugStopWatch<Debug> timer("Optimize concat");
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
                                        block->statements.insert(block->statements.begin(), std::make_shared<tac::Quadruple>(ret1, label));
                                        block->statements.insert(block->statements.begin()+1, std::make_shared<tac::Quadruple>(ret2, length));

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
    DebugStopWatch<Debug> timer("Remove needless jumps");
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
    DebugStopWatch<Debug> timer("Merge basic blocks");
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
        optimized |= debug<Debug, 4>(apply_to_basic_blocks<ConstantPropagation>(program));

        //Offset Constant propagation
        optimized |= debug<Debug, 5>(apply_to_basic_blocks<OffsetConstantPropagation>(program));
        
        //Copy propagation
        optimized |= debug<Debug, 6>(apply_to_basic_blocks<CopyPropagation>(program));
        
        //Offset Copy propagation
        optimized |= debug<Debug, 7>(apply_to_basic_blocks<OffsetCopyPropagation>(program));

        //Propagate math
        optimized |= debug<Debug, 8>(apply_to_basic_blocks_two_pass<MathPropagation>(program));

        //Remove unused assignations
        optimized |= debug<Debug, 9>(apply_to_basic_blocks_two_pass<RemoveAssign>(program));

        //Remove unused assignations
        optimized |= debug<Debug, 10>(apply_to_basic_blocks_two_pass<RemoveMultipleAssign>(program));
       
        //Optimize concatenations 
        optimized |= debug<Debug, 11>(optimize_concat(program, pool));

        //Remove dead basic blocks (unreachable code)
        optimized |= debug<Debug, 12>(remove_dead_basic_blocks(program));

        //Remove needless jumps
        optimized |= debug<Debug, 13>(remove_needless_jumps(program));

        //Merge basic blocks
        optimized |= debug<Debug, 14>(merge_basic_blocks(program));
    } while (optimized);
}
