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

mtac::BinaryOperator to_binary_operator(mtac::Operator op){
    switch(op){ 
        /* relational operators */
        case mtac::Operator::EQUALS:
            return mtac::BinaryOperator::EQUALS;
        case mtac::Operator::NOT_EQUALS:
            return mtac::BinaryOperator::NOT_EQUALS;
        case mtac::Operator::GREATER:
            return mtac::BinaryOperator::GREATER;
        case mtac::Operator::GREATER_EQUALS:
            return mtac::BinaryOperator::GREATER_EQUALS;
        case mtac::Operator::LESS:
            return mtac::BinaryOperator::LESS;
        case mtac::Operator::LESS_EQUALS:
            return mtac::BinaryOperator::LESS_EQUALS;

            /* float relational operators */
        case mtac::Operator::FE:
            return mtac::BinaryOperator::FE;
        case mtac::Operator::FNE:
            return mtac::BinaryOperator::FNE;
        case mtac::Operator::FG:
            return mtac::BinaryOperator::FG;
        case mtac::Operator::FGE:
            return mtac::BinaryOperator::FGE;
        case mtac::Operator::FLE:
            return mtac::BinaryOperator::FLE;
        case mtac::Operator::FL:
            return mtac::BinaryOperator::FL;

        default:
            eddic_unreachable("Not a binary operator");
    }
}

template<typename Branch>
bool optimize_branch(std::shared_ptr<Branch> branch, mtac::basic_block_p basic_block, mtac::VariableUsage variable_usage){
    if(!branch->op && mtac::isVariable(branch->arg1)){
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
                    branch->op = to_binary_operator(declaration->op);

                    return true;
                }
            }
        }
    }

    return false;
}

} //end of anonymous namespace

bool mtac::conditional_propagation::operator()(mtac::function_p function){
    bool optimized = false;

    auto variable_usage = mtac::compute_variable_usage(function);

    for(auto& basic_block : function){
        for(auto& statement : basic_block){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::IfFalse>>(&statement)){
                optimized |= optimize_branch(*ptr, basic_block, variable_usage);
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::If>>(&statement)){
                optimized |= optimize_branch(*ptr, basic_block, variable_usage);
            }
        }
    }

    return optimized;
}
