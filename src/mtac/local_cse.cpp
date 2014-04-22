//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "FunctionContext.hpp"
#include "GlobalContext.hpp"
#include "Variable.hpp"

#include "mtac/local_cse.hpp"
#include "mtac/cse.hpp"
#include "mtac/Function.hpp"
#include "mtac/Quadruple.hpp"
#include "mtac/Utils.hpp"

using namespace eddic;

bool mtac::local_cse::operator()(mtac::Function& function){
    auto escaped = mtac::escape_analysis(function);
    bool optimized = false;
    
    for(auto& block : function){
        auto it = block->statements.begin();

        std::vector<mtac::expression> expressions;

        while(it != block->statements.end()){
            auto& quadruple = *it;
                
            if(mtac::is_expression(quadruple.op) && mtac::is_interesting(quadruple) && mtac::is_valid(quadruple, *escaped)){
                bool found = false;

                for(auto& exp : expressions){
                    if(are_equivalent(quadruple, exp)){
                        found = true;
                                
                        function.context->global()->stats().inc_counter("local_cse");

                        optimized = true;
                            
                        mtac::Operator op = mtac::assign_op(exp.op);

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
            
            mtac::kill_expressions(*it, expressions);

            ++it;
        }
    }

    return optimized;
}
