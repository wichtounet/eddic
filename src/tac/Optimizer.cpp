//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <memory>
#include <unordered_map>

#include <boost/variant.hpp>

#include "tac/Optimizer.hpp"
#include "tac/Program.hpp"
#include "tac/Utils.hpp"

using namespace eddic;

namespace {

struct ArithmeticIdentities : public boost::static_visitor<tac::Statement> {
    bool optimized;

    ArithmeticIdentities() : optimized(false) {}

    tac::Statement operator()(std::shared_ptr<tac::Quadruple>& quadruple){
        bool old = optimized;
        optimized = true;

        if(quadruple->op){
            switch(*quadruple->op){
                case tac::Operator::ADD:
                    if(tac::equals<int>(quadruple->arg1, 0)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, *quadruple->arg2);
                    } else if(tac::equals<int>(*quadruple->arg2, 0)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, quadruple->arg1);
                    }

                    break;
                case tac::Operator::SUB:
                    if(tac::equals<int>(*quadruple->arg2, 0)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, quadruple->arg1);
                    } 

                    //a = b - b => a = 0
                    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&quadruple->arg1)){
                        if(tac::equals<std::shared_ptr<Variable>>(*quadruple->arg2, *ptr)){
                            return std::make_shared<tac::Quadruple>(quadruple->result, 0);
                        }
                    }

                    //TODO Transform x = 0 - a into x = -a by adding a NEG tac instruction

                    break;
                case tac::Operator::MUL:
                    if(tac::equals<int>(quadruple->arg1, 1)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, *quadruple->arg2);
                    } else if(tac::equals<int>(*quadruple->arg2, 1)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, quadruple->arg1);
                    }
                    
                    if(tac::equals<int>(quadruple->arg1, 0)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, 0);
                    } else if(tac::equals<int>(*quadruple->arg2, 0)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, 0);
                    }

                    //TODO Transform x = a * -1 and x = -1 * a into x = -a by adding a NEG tac instruction

                    break;
                case tac::Operator::DIV:
                    if(tac::equals<int>(*quadruple->arg2, 1)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, quadruple->arg1);
                    }

                    if(tac::equals<int>(quadruple->arg1, 0)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, 0);
                    }

                    //a = b / b => a = 1
                    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&quadruple->arg1)){
                        if(tac::equals<std::shared_ptr<Variable>>(*quadruple->arg2, *ptr)){
                            return std::make_shared<tac::Quadruple>(quadruple->result, 1);
                        }
                    }

                    //TODO Transform x = 0 / -1 into x = -a by adding a NEG tac instruction

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
                    if(tac::equals<int>(quadruple->arg1, 2)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, *quadruple->arg2, tac::Operator::ADD, *quadruple->arg2);
                    } else if(tac::equals<int>(*quadruple->arg2, 2)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, quadruple->arg1, tac::Operator::ADD, quadruple->arg1);
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
                    if(tac::isInt(quadruple->arg1) && tac::isInt(*quadruple->arg2)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, 
                            boost::get<int>(quadruple->arg1) + boost::get<int>(*quadruple->arg2));
                    }

                    break;
                case tac::Operator::SUB:
                    if(tac::isInt(quadruple->arg1) && tac::isInt(*quadruple->arg2)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, 
                            boost::get<int>(quadruple->arg1) - boost::get<int>(*quadruple->arg2));
                    }

                    break;
                case tac::Operator::MUL:
                    if(tac::isInt(quadruple->arg1) && tac::isInt(*quadruple->arg2)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, 
                            boost::get<int>(quadruple->arg1) * boost::get<int>(*quadruple->arg2));
                    }

                    break;
                case tac::Operator::DIV:
                    if(tac::isInt(quadruple->arg1) && tac::isInt(*quadruple->arg2)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, 
                            boost::get<int>(quadruple->arg1) / boost::get<int>(*quadruple->arg2));
                    }

                    break;
                case tac::Operator::MOD:
                    if(tac::isInt(quadruple->arg1) && tac::isInt(*quadruple->arg2)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, 
                            boost::get<int>(quadruple->arg1) % boost::get<int>(*quadruple->arg2));
                    }

                    break;
                default:
                    break;
            }
        }

        optimized = old;
        return quadruple;
    }

    tac::Statement operator()(std::shared_ptr<tac::IfFalse>& ifFalse){
        //TODO Evaluate boolean expressions at compile time

        return ifFalse;
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
        bool old = optimized;
        optimized = true;

        if(!quadruple->op){
            if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&quadruple->arg1)){
                if(constants.find(*ptr) != constants.end()){
                    quadruple->arg1 = constants[*ptr];
                }
            }

            if(auto* ptr = boost::get<int>(&quadruple->arg1)){
                constants[quadruple->result] = *ptr;
            }
        } else {
            if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&quadruple->arg1)){
                if(constants.find(*ptr) != constants.end()){
                    quadruple->arg1 = constants[*ptr];
                }
            }

            if(quadruple->arg2){
                if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg2)){
                    if(constants.find(*ptr) != constants.end()){
                        quadruple->arg2 = constants[*ptr];
                    }
                }
            }

            //The result is not constant at this point
            constants.erase(quadruple->result);
        }

        optimized = old;
        return quadruple;
    }

    template<typename T>
        tac::Statement operator()(T& statement){ 
            return statement;
        }
};

template<typename Visitor>
bool apply_to_all(tac::Program& program){
    Visitor visitor;

    for(auto& function : program.functions){
        for(auto& block : function->getBasicBlocks()){
            for(auto& statement : block->statements){
                statement = boost::apply_visitor(visitor, statement);
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
                statement = boost::apply_visitor(visitor, statement);
            }

            optimized |= visitor.optimized;
        }
    }

    return optimized;
}

}

void tac::Optimizer::optimize(tac::Program& program) const {
    bool optimized;
    do {
        optimized = false;

        //Optimize using arithmetic identities
        optimized |= apply_to_all<ArithmeticIdentities>(program);
        
        //Reduce arithtmetic instructions in strength
        optimized |= apply_to_all<ReduceInStrength>(program);

        //Constant folding
        optimized |= apply_to_all<ConstantFolding>(program);

        //Constant propagation
        optimized |= apply_to_basic_blocks<ConstantPropagation>(program);
    } while (optimized);
   
    //TODO Remove unused temporaries
    
    //TODO Copy propagation
}
