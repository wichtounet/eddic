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

namespace {

bool optimize_dot(std::shared_ptr<mtac::Quadruple> quadruple, mtac::Operator op, std::unordered_map<std::shared_ptr<Variable>, std::shared_ptr<Variable>>& aliases){
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
        auto variable = *ptr;

        if(aliases.find(variable) != aliases.end()){
            auto alias = aliases[variable];

            quadruple->arg1 = alias;

            if(auto* offset_ptr = boost::get<int>(&*quadruple->arg2)){
                if(*offset_ptr == 0){
                    quadruple->op = op;
                    quadruple->arg2.reset();
                }
            }

            return true;
        }
    }

    return false;
}

bool optimize_dot_assign(std::shared_ptr<mtac::Quadruple> quadruple, mtac::Operator op, std::unordered_map<std::shared_ptr<Variable>, std::shared_ptr<Variable>>& aliases){
    auto variable = quadruple->result;

    if(aliases.find(variable) != aliases.end()){
        auto alias = aliases[variable];

        quadruple->result = alias;

        if(auto* offset_ptr = boost::get<int>(&*quadruple->arg1)){
            if(*offset_ptr == 0){
                quadruple->op = op;
                quadruple->arg1.reset();
                quadruple->arg1 = quadruple->arg2;
            }
        }

        return true;
    }

    return false;
}

} //end of anonymous namespace

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
        optimized |= optimize_dot(quadruple, mtac::Operator::ASSIGN, aliases);
    } else if(quadruple->op == mtac::Operator::FDOT){
        optimized |= optimize_dot(quadruple, mtac::Operator::FASSIGN, aliases);
    } else if(quadruple->op == mtac::Operator::DOT_ASSIGN){
        optimized |= optimize_dot_assign(quadruple, mtac::Operator::ASSIGN, aliases);
    } else if(quadruple->op == mtac::Operator::DOT_FASSIGN){
        optimized |= optimize_dot_assign(quadruple, mtac::Operator::FASSIGN, aliases);
    }
}

void mtac::PointerPropagation::operator()(std::shared_ptr<mtac::Param> /*param*/){
    //Nothing to check
}
