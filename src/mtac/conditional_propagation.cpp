//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Variable.hpp"

#include "mtac/conditional_propagation.hpp"
#include "mtac/Function.hpp"
#include "mtac/Statement.hpp"
#include "mtac/Utils.hpp"

using namespace eddic;

namespace {

std::shared_ptr<mtac::Quadruple> get_variable_declaration(mtac::basic_block_p basic_block, std::shared_ptr<Variable> variable){
    for(auto& statement : basic_block){
        if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
            if((*ptr)->result == variable){
                return *ptr;
            }
        }
    }

    return nullptr;
}

template<bool If>
mtac::Operator to_binary_operator(mtac::Operator op){
    if(If){
        switch(op){ 
            /* relational operators */
            case mtac::Operator::EQUALS:
                return mtac::Operator::IF_EQUALS;
            case mtac::Operator::NOT_EQUALS:
                return mtac::Operator::IF_NOT_EQUALS;
            case mtac::Operator::GREATER:
                return mtac::Operator::IF_GREATER;
            case mtac::Operator::GREATER_EQUALS:
                return mtac::Operator::IF_GREATER_EQUALS;
            case mtac::Operator::LESS:
                return mtac::Operator::IF_LESS;
            case mtac::Operator::LESS_EQUALS:
                return mtac::Operator::IF_LESS_EQUALS;

            /* float relational operators */
            case mtac::Operator::FE:
                return mtac::Operator::IF_FE;
            case mtac::Operator::FNE:
                return mtac::Operator::IF_FNE;
            case mtac::Operator::FG:
                return mtac::Operator::IF_FG;
            case mtac::Operator::FGE:
                return mtac::Operator::IF_FGE;
            case mtac::Operator::FLE:
                return mtac::Operator::IF_FLE;
            case mtac::Operator::FL:
                return mtac::Operator::IF_FL;

            default:
                eddic_unreachable("Not a binary operator");
        }
    } else {
        switch(op){ 
            /* relational operators */
            case mtac::Operator::EQUALS:
                return mtac::Operator::IF_FALSE_EQUALS;
            case mtac::Operator::NOT_EQUALS:
                return mtac::Operator::IF_FALSE_NOT_EQUALS;
            case mtac::Operator::GREATER:
                return mtac::Operator::IF_FALSE_GREATER;
            case mtac::Operator::GREATER_EQUALS:
                return mtac::Operator::IF_FALSE_GREATER_EQUALS;
            case mtac::Operator::LESS:
                return mtac::Operator::IF_FALSE_LESS;
            case mtac::Operator::LESS_EQUALS:
                return mtac::Operator::IF_FALSE_LESS_EQUALS;

            /* float relational operators */
            case mtac::Operator::FE:
                return mtac::Operator::IF_FALSE_FE;
            case mtac::Operator::FNE:
                return mtac::Operator::IF_FALSE_FNE;
            case mtac::Operator::FG:
                return mtac::Operator::IF_FALSE_FG;
            case mtac::Operator::FGE:
                return mtac::Operator::IF_FALSE_FGE;
            case mtac::Operator::FLE:
                return mtac::Operator::IF_FALSE_FLE;
            case mtac::Operator::FL:
                return mtac::Operator::IF_FALSE_FL;

            default:
                eddic_unreachable("Not a binary operator");
        }
    }
}

template<bool If, typename Branch>
bool optimize_branch(std::shared_ptr<Branch> branch, mtac::basic_block_p basic_block, mtac::VariableUsage variable_usage){
    if(mtac::isVariable(branch->arg1)){
        auto variable = boost::get<std::shared_ptr<Variable>>(branch->arg1);

        auto declaration = get_variable_declaration(basic_block, variable);

        if(variable_usage[variable] == 2){
            auto declaration = get_variable_declaration(basic_block, variable);

            if(declaration){
                if(
                        (declaration->op >= mtac::Operator::EQUALS && declaration->op <= mtac::Operator::LESS_EQUALS)
                        ||  (declaration->op >= mtac::Operator::FE && declaration->op <= mtac::Operator::FL)){
                    branch->arg1 = *declaration->arg1;
                    branch->arg2 = *declaration->arg2;
                    branch->op = to_binary_operator<If>(declaration->op);

                    return true;
                }
            }
        }
    }

    return false;
}

} //end of anonymous namespace

bool mtac::conditional_propagation::operator()(mtac::Function& function){
    bool optimized = false;

    auto variable_usage = mtac::compute_variable_usage(function);

    for(auto& basic_block : function){
        for(auto& statement : basic_block){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::IfFalse>>(&statement)){
                if((*ptr)->op == mtac::Operator::IF_FALSE_UNARY){
                    optimized |= optimize_branch<false>(*ptr, basic_block, variable_usage);
                }
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::If>>(&statement)){
                if((*ptr)->op == mtac::Operator::IF_UNARY){
                    optimized |= optimize_branch<true>(*ptr, basic_block, variable_usage);
                }
            }
        }
    }

    return optimized;
}
