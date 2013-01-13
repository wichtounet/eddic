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

#include "mtac/CommonSubexpressionElimination.hpp"
#include "mtac/Utils.hpp"
#include "mtac/Printer.hpp"
#include "mtac/Quadruple.hpp"

#include "ltac/Statement.hpp"

using namespace eddic;

typedef mtac::CommonSubexpressionElimination::ProblemDomain ProblemDomain;

std::ostream& mtac::operator<<(std::ostream& os, Expression& expression){
    mtac::Printer printer;
    os << "Expression {expression = ";
    printer.print_inline(expression.expression, os);
    return os << "}";
}

inline bool are_equivalent(std::shared_ptr<mtac::Quadruple> first, std::shared_ptr<mtac::Quadruple> second){
    return first->op == second->op && *first->arg1 == *second->arg1 && *first->arg2 == *second->arg2;
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
                if(are_equivalent(in_value.expression, out_value.expression)){
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

ProblemDomain mtac::CommonSubexpressionElimination::transfer(mtac::basic_block_p basic_block, std::shared_ptr<mtac::Quadruple>& quadruple, ProblemDomain& in){
    auto out = in;
    auto op = quadruple.op;

    if(mtac::is_expression(op)){
        bool exists = false;
        for(auto& expression : out.values()){
            if(are_equivalent(quadruple, expression.expression)){
                exists = true;
                break;
            }
        }

        if(!exists){
            Expression expression;
            expression.expression = quadruple;
            expression.source = basic_block;

            out.values().push_back(expression);
        }
    }

    if(mtac::erase_result(op)){
        auto it = iterate(out.values());

        while(it.has_next()){
            auto& expression = (*it).expression;

            if(expression->arg1){
                if(auto* var_ptr = boost::get<std::shared_ptr<Variable>>(&*expression->arg1)){
                    if(*var_ptr == quadruple.result){
                        it.erase();
                        continue;
                    }
                }
            }

            if(expression->arg2){
                if(auto* var_ptr = boost::get<std::shared_ptr<Variable>>(&*expression->arg2)){
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
                    if(are_equivalent(quadruple, expression.expression)){
                        exists = true;
                        break;
                    }
                }

                if(!exists){
                    Expression expression;
                    expression.expression = quadruple;
                    expression.source = block;

                    values.push_back(expression);
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
        for(auto quadruple : block->statements){
            auto& results = global_results->IN_S[quadruple];

            if(results.top()){
                continue;
            }

            if(mtac::is_expression(quadruple.op)){
                for(auto& expression : results.values()){
                    auto source_statement = expression.expression;

                    if(are_equivalent(source_statement, quadruple)){
                        mtac::Operator assign_op;
                        if(quadruple.op >= mtac::Operator::ADD && quadruple.op <= mtac::Operator::MOD){
                            assign_op = mtac::Operator::ASSIGN;
                        } else {
                            assign_op = mtac::Operator::FASSIGN;
                        } 

                        if(optimized.find(source_statement) == optimized.end()){
                            std::shared_ptr<Variable> temp;
                            if(quadruple.op >= mtac::Operator::ADD && quadruple.op <= mtac::Operator::MOD){
                                temp = expression.source->context->new_temporary(INT);
                            } else {
                                temp = expression.source->context->new_temporary(FLOAT);
                            } 

                            auto it = expression.source->statements.begin();
                            auto end = expression.source->statements.end();

                            while(it != end){
                                auto target = *it;
                                if(target == source_statement){
                                    ++it;
                                    expression.source->statements.insert(it, std::make_shared<mtac::Quadruple>(source_statement->result, temp, assign_op));

                                    break;
                                }

                                ++it;
                            }

                            source_statement->result = temp;

                            optimized.insert(source_statement);
                        }

                        if(optimized.find(quadruple) == optimized.end()){
                            quadruple.op = assign_op;
                            quadruple.arg1 = source_statement->result;
                            quadruple.arg2.reset();

                            optimized.insert(quadruple);

                            changes = true;
                        }
                    }
                }
            }
        }
    }

    return changes;
}
