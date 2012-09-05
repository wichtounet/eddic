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
#include "VisitorUtils.hpp"

using namespace eddic;

namespace {

bool optimize_dot(std::shared_ptr<mtac::Quadruple> quadruple, mtac::Operator op, std::unordered_map<std::shared_ptr<Variable>, std::shared_ptr<Variable>>& aliases){
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
        auto variable = *ptr;

        if(aliases.count(variable)){
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

    if(aliases.count(variable)){
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

struct CopyApplier : public boost::static_visitor<> {
    std::unordered_map<std::shared_ptr<Variable>, std::shared_ptr<Variable>>& pointer_copies;
    bool changes = false;

    CopyApplier(std::unordered_map<std::shared_ptr<Variable>, std::shared_ptr<Variable>>& pointer_copies) : pointer_copies(pointer_copies) {}

    bool optimize_arg(mtac::Argument& arg){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&arg)){
            if(pointer_copies.count(*ptr)){
                arg = pointer_copies[*ptr];
                return true;
            }
        }

        return false;
    }

    bool optimize_optional(boost::optional<mtac::Argument>& arg){
        if(arg){
            return optimize_arg(*arg);
        }

        return false;
    }

    void operator()(std::shared_ptr<mtac::Quadruple> quadruple){
        changes |= optimize_optional(quadruple->arg1);
        changes |= optimize_optional(quadruple->arg2);
    }

    void operator()(std::shared_ptr<mtac::Param> param){
        changes |= optimize_arg(param->arg);
    }
};

} //end of anonymous namespace

void mtac::PointerPropagation::operator()(std::shared_ptr<mtac::Quadruple> quadruple){
    CopyApplier optimizer(pointer_copies);
    visit_non_variant(optimizer, quadruple);

    optimized |= optimizer.changes;

    if(mtac::erase_result(quadruple->op)){
        aliases.erase(quadruple->result);
        pointer_copies.erase(quadruple->result);
    }

    if(quadruple->op == mtac::Operator::PASSIGN){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
            if((*ptr)->type()->is_standard_type()){
                aliases[quadruple->result] = *ptr;
            } else if((*ptr)->type()->is_pointer()){
                if(aliases.count(*ptr)){
                    auto alias = aliases[*ptr];
                    quadruple->arg1 = alias;

                    optimized = true;
                }
            }
        }
        
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
            if((*ptr)->type()->is_pointer() && quadruple->result->type()->is_pointer()){
                pointer_copies[quadruple->result] = *ptr;    
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

void mtac::PointerPropagation::operator()(std::shared_ptr<mtac::Param> param){
    CopyApplier optimizer(pointer_copies);
    visit_non_variant(optimizer, param);
    
    optimized |= optimizer.changes;
}
