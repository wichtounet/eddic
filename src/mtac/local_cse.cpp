//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "FunctionContext.hpp"
#include "GlobalContext.hpp"
#include "Variable.hpp"

#include "mtac/Argument.hpp"
#include "mtac/local_cse.hpp"
#include "mtac/Function.hpp"
#include "mtac/Quadruple.hpp"
#include "mtac/Utils.hpp"

using namespace eddic;

namespace {

struct expression {
    std::size_t uid;
    mtac::Argument arg1;
    mtac::Argument arg2;
    mtac::Operator op;
    std::shared_ptr<Variable> tmp;
    std::shared_ptr<const Type> type;

    expression(std::size_t uid, mtac::Argument arg1, mtac::Argument arg2, mtac::Operator op, std::shared_ptr<Variable> tmp, std::shared_ptr<const Type> type) : uid(uid), arg1(arg1), arg2(arg2), op(op), tmp(tmp), type(type) {
        //Nothing
    }
};

bool is_interesting(mtac::Quadruple& quadruple){
    if(quadruple.op == mtac::Operator::DOT){
        auto var = boost::get<std::shared_ptr<Variable>>(*quadruple.arg1);

        return !var->type()->is_pointer();
    }

    if(boost::get<std::shared_ptr<Variable>>(&*quadruple.arg1)){
        return true;
    }
    
    if(boost::get<std::shared_ptr<Variable>>(&*quadruple.arg2)){
        return true;
    }

    return false;
}

bool are_equivalent(mtac::Quadruple& quadruple, expression& exp){
    if(exp.op == quadruple.op && exp.type == quadruple.result->type()){
        if(exp.arg1 == *quadruple.arg1 && exp.arg2 == *quadruple.arg2){
            return true;
        } else if(mtac::is_distributive(quadruple.op) && exp.arg1 == *quadruple.arg2 && exp.arg2 == *quadruple.arg1){
            return true;
        }
    }

    return false;
}

}

bool mtac::local_cse::operator()(mtac::Function& function){
    bool optimized = false;
    
    for(auto& block : function){
        auto it = block->statements.begin();

        std::vector<expression> expressions;

        while(it != block->statements.end()){
            auto& quadruple = *it;
                
            if(mtac::is_expression(quadruple.op) && is_interesting(quadruple)){
                bool found = false;

                for(auto& exp : expressions){
                    if(are_equivalent(quadruple, exp)){
                        found = true;
                                
                        function.context->global()->stats().inc_counter("local_cse");

                        optimized = true;
                            
                        mtac::Operator op;
                        if(exp.op == mtac::Operator::DOT){
                            op = mtac::Operator::ASSIGN;
                        } else if(exp.op <= mtac::Operator::ADD && exp.op <= mtac::Operator::MOD){
                            op = mtac::Operator::ASSIGN;
                        } else if(exp.op <= mtac::Operator::FADD && exp.op <= mtac::Operator::FDIV){
                            op = mtac::Operator::FASSIGN;
                        }

                        if(!exp.tmp){
                            auto tmp = function.context->new_temporary(exp.type);
                            exp.tmp = tmp;

                            auto current_uid = quadruple.uid();

                            quadruple.op = op;
                            quadruple.arg1 = tmp;
                            quadruple.arg2.reset();

                            auto old_it = block->statements.begin();
                            while(old_it->uid() != exp.uid){
                                ++old_it;
                            }

                            old_it->op = op;
                            old_it->arg1 = tmp;
                            old_it->arg2.reset();

                            block->statements.insert(old_it, mtac::Quadruple(tmp, exp.arg1, exp.op, exp.arg2));

                            it = block->statements.begin();
                            while(it->uid() != current_uid){
                                ++it;
                            }
                        } else {
                            quadruple.op = op;
                            quadruple.arg1 = exp.tmp;
                            quadruple.arg2.reset();
                        }

                        break;
                    }
                }

                if(!found){
                    expressions.emplace_back(quadruple.uid(), *quadruple.arg1, *quadruple.arg2, quadruple.op, 
                            nullptr, quadruple.result->type());
                }
            }
            
            if(mtac::erase_result(quadruple.op)){
                auto eit = iterate(expressions);

                while(eit.has_next()){
                    auto& expression = *eit;

                    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&expression.arg1)){
                        if(quadruple.result == *ptr){
                            eit.erase();
                            continue;
                        }
                    }
                    
                    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&expression.arg2)){
                        if(quadruple.result == *ptr){
                            eit.erase();
                            continue;
                        }
                    }

                    ++eit;
                }
            }

            ++it;
        }
    }

    return optimized;
}
