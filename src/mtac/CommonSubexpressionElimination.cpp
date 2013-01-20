//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "Variable.hpp"
#include "FunctionContext.hpp"
#include "Type.hpp"
#include "iterators.hpp"
#include "GlobalContext.hpp"

#include "mtac/CommonSubexpressionElimination.hpp"
#include "mtac/Utils.hpp"
#include "mtac/Quadruple.hpp"

#include "ltac/Statement.hpp"

using namespace eddic;

typedef mtac::CommonSubexpressionElimination::ProblemDomain ProblemDomain;

std::ostream& mtac::operator<<(std::ostream& os, const Expression& expression){
    return os << "Expression {expression = " << expression.expression;
}

inline bool are_equivalent(mtac::Quadruple& first, mtac::Quadruple& second){
    return first.op == second.op && *first.arg1 == *second.arg1 && *first.arg2 == *second.arg2;
}

void mtac::CommonSubexpressionElimination::meet(ProblemDomain& in, const ProblemDomain& out){
    eddic_assert(!in.top() || !out.top(), "At least one lattice should not be a top element");

    if(in.top()){
        in = out;
    } else if(out.top()){
        //in does not change
    } else {
        auto it = iterate(in.values());

        while(it.has_next()){
            auto& in_value = *it;
            bool found = false;

            for(auto& out_value : out.values()){
                if(are_equivalent(function->find(in_value.expression), function->find(out_value.expression))){
                    found = true;
                    break;
                }
            }

            if(!found){
                it.erase();
                continue;
            }

            ++it;
        }
    }
}

ProblemDomain mtac::CommonSubexpressionElimination::transfer(mtac::basic_block_p basic_block, mtac::Quadruple& quadruple, ProblemDomain& in){
    auto out = in;
    auto op = quadruple.op;

    if(mtac::is_expression(op)){
        bool exists = false;
        for(auto& expression : out.values()){
            if(are_equivalent(quadruple, function->find(expression.expression))){
                exists = true;
                break;
            }
        }

        if(!exists){
            out.values().push_back({quadruple.uid(), basic_block});
        }
    }

    if(mtac::erase_result(op)){
        auto it = iterate(out.values());

        while(it.has_next()){
            auto& expression = function->find((*it).expression);

            if(expression.arg1){
                if(auto* var_ptr = boost::get<std::shared_ptr<Variable>>(&*expression.arg1)){
                    if(*var_ptr == quadruple.result){
                        it.erase();
                        continue;
                    }
                }
            }

            if(expression.arg2){
                if(auto* var_ptr = boost::get<std::shared_ptr<Variable>>(&*expression.arg2)){
                    if(*var_ptr == quadruple.result){
                        it.erase();
                        continue;
                    }
                }
            }

            ++it;
        }
    }

    return out;
}

ProblemDomain mtac::CommonSubexpressionElimination::Boundary(mtac::Function& /*function*/){
    return default_element();
}

ProblemDomain mtac::CommonSubexpressionElimination::Init(mtac::Function& function){
    this->function = &function;

    if(init){
        ProblemDomain result(*init);
        return result;
    }

    typename ProblemDomain::Values values;
    
    for(auto& block : function){
        for(auto& quadruple : block->statements){
            if(mtac::is_expression(quadruple.op)){
                bool exists = false;
                for(auto& expression : values){
                    if(are_equivalent(quadruple, function.find(expression.expression))){
                        exists = true;
                        break;
                    }
                }

                if(!exists){
                    values.push_back({quadruple.uid(), block});
                }
            }
        }
    }

    init = values;
    
    ProblemDomain result(values);
    return result;
}

bool mtac::CommonSubexpressionElimination::optimize(mtac::Function& function, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>> global_results){
    bool changes = false;

    for(auto& block : function){
        auto qit = block->statements.begin();
        auto qend = block->statements.end();

        while(qit != qend){
            bool local = false;
            auto& quadruple = *qit;
            auto& results = global_results->IN_S[quadruple.uid()];

            if(results.top()){
                ++qit;
                continue;
            }

            if(optimized.find(quadruple.uid()) == optimized.end()){
                if(mtac::is_expression(quadruple.op)){
                    for(auto& expression : results.values()){
                        auto& source_statement = function.find(expression.expression);
                        auto result = source_statement.result;
                        auto quid = quadruple.uid();

                        if(are_equivalent(source_statement, quadruple)){
                            mtac::Operator assign_op;
                            if(quadruple.op >= mtac::Operator::ADD && quadruple.op <= mtac::Operator::MOD){
                                assign_op = mtac::Operator::ASSIGN;
                            } else {
                                assign_op = mtac::Operator::FASSIGN;
                            } 

                            std::shared_ptr<Variable> new_result = result;

                            if(optimized.find(source_statement.uid()) == optimized.end()){
                                function.context->global()->stats().inc_counter("common_subexpr_eliminated");

                                std::shared_ptr<Variable> temp;
                                if(quadruple.op >= mtac::Operator::ADD && quadruple.op <= mtac::Operator::MOD){
                                    temp = expression.source->context->new_temporary(INT);
                                } else {
                                    temp = expression.source->context->new_temporary(FLOAT);
                                } 

                                new_result = temp;

                                auto it = expression.source->statements.begin();
                                auto end = expression.source->statements.end();

                                source_statement.result = temp;

                                optimized.insert(source_statement.uid());

                                while(it != end){
                                    auto& target = *it;
                                    if(target == source_statement){
                                        ++it;
                                        expression.source->statements.insert(it, mtac::Quadruple(result, temp, assign_op));

                                        break;
                                    }

                                    ++it;
                                }
                            }

                            if(optimized.find(quid) == optimized.end()){
                                function.context->global()->stats().inc_counter("common_subexpr_eliminated");

                                auto& quadruple = function.find(quid);

                                eddic_assert(new_result, "Should have been filled");

                                quadruple.op = assign_op;
                                quadruple.arg1 = new_result;
                                quadruple.arg2.reset();

                                optimized.insert(quid);

                                local = true;
                                changes = true;

                                break;
                            }
                        }
                    }
                }
            }

            if(local){
                qit = block->statements.begin();
                qend = block->statements.end();
            } else {
                ++qit;
            }
        }
    }

    return changes;
}
