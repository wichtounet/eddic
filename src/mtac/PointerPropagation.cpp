//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "mtac/PointerPropagation.hpp"
#include "mtac/OptimizerUtils.hpp"
#include "mtac/Utils.hpp"

#include "Variable.hpp"
#include "Type.hpp"

using namespace eddic;

void mtac::PointerPropagation::operator()(std::shared_ptr<mtac::Quadruple> quadruple){
    if(mtac::erase_result(quadruple->op)){
        aliases.erase(quadruple->result);
    }

    if(quadruple->op == mtac::Operator::PASSIGN){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
            if((*ptr)->type()->is_standard_type()){
                aliases[quadruple->result] = *ptr;
            } else if((*ptr)->type()->is_pointer()){
                if(aliases.find(*ptr) != aliases.end()){
                    auto alias = aliases[*ptr];
                    quadruple->arg1 = alias;

                    optimized = true;
                }
            }
        }
    } else if(quadruple->op == mtac::Operator::DOT){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
            auto variable = *ptr;

            if(aliases.find(variable) != aliases.end()){
                auto alias = aliases[variable];

                quadruple->arg1 = alias;

                if(auto* offset_ptr = boost::get<int>(&*quadruple->arg2)){
                    if(*offset_ptr == 0){
                        quadruple->op = mtac::Operator::ASSIGN;
                        quadruple->arg2.reset();
                    }
                }

                optimized = true;
            }
        }
    } else if(quadruple->op == mtac::Operator::DOT_ASSIGN){
        auto variable = quadruple->result;

        if(aliases.find(variable) != aliases.end()){
            auto alias = aliases[variable];

            quadruple->result = alias;

            if(auto* offset_ptr = boost::get<int>(&*quadruple->arg1)){
                if(*offset_ptr == 0){
                    quadruple->op = mtac::Operator::ASSIGN;
                    quadruple->arg1.reset();
                    quadruple->arg1 = quadruple->arg2;
                }
            }

            optimized = true;
        }
    }
    
    //DOT_ASSIGN,     //result+arg1=arg2
    //DOT_FASSIGN,    //result+arg1=arg2
    //DOT_PASSIGN,    //result+arg1=arg2
}

void mtac::PointerPropagation::operator()(std::shared_ptr<mtac::Param> param){
   /* if(param->address){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&param->arg)){
            
        }
    }*/
}
