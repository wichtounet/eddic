//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <memory>

#include <boost/variant.hpp>

#include "tac/Optimizer.hpp"
#include "tac/Program.hpp"
#include "tac/Utils.hpp"

using namespace eddic;

namespace {

struct ArithmeticIdentities : public boost::static_visitor<tac::Statement> {
    tac::Statement operator()(std::shared_ptr<tac::Quadruple>& quadruple){
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

                    //TODO Transform x = 0 - a into x = -a by adding a NEG tac instruction

                    break;
                case tac::Operator::MUL:
                    if(tac::equals<int>(quadruple->arg1, 1)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, *quadruple->arg2);
                    } else if(tac::equals<int>(*quadruple->arg2, 1)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, quadruple->arg1);
                    }

                    //TODO Transform x = a * -1 and x = -1 * a into x = -a by adding a NEG tac instruction

                    break;
                case tac::Operator::DIV:
                    if(tac::equals<int>(*quadruple->arg2, 1)){
                        return std::make_shared<tac::Quadruple>(quadruple->result, quadruple->arg1);
                    }

                    //TODO Transform x = 0 / -1 into x = -a by adding a NEG tac instruction

                    break;
                default:
                    return quadruple;
            }
        }

        return quadruple;
    }

    template<typename T>
    tac::Statement operator()(T& statement) const { 
        return statement;
    }
};
    
}

template<typename Visitor>
void apply_to_all(Visitor visitor, tac::Program& program){
    for(auto& function : program.functions){
        for(auto& block : function->getBasicBlocks()){
            for(auto& statement : block->statements){
                statement = boost::apply_visitor(visitor, statement);
            }
        }
    }
}

void tac::Optimizer::optimize(tac::Program& program) const {
    ArithmeticIdentities identities;
    apply_to_all(identities, program);
}
