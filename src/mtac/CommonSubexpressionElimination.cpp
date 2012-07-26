//=======================================================================
// Copyright Baptiste Wicht 2011.
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

using namespace eddic;

typedef mtac::CommonSubexpressionElimination::ProblemDomain ProblemDomain;

std::ostream& mtac::operator<<(std::ostream& stream, Expression& /*expression*/){
    return stream << "Expression {expression = {}}";
}

inline bool are_equivalent(std::shared_ptr<mtac::Quadruple> first, std::shared_ptr<mtac::Quadruple> second){
    return first->op == second->op && *first->arg1 == *second->arg1 && *first->arg2 == *second->arg2;
}

ProblemDomain mtac::CommonSubexpressionElimination::meet(ProblemDomain& in, ProblemDomain& out){
    ASSERT(!in.top() || !out.top(), "At least one lattice should not be a top element");

    if(in.top()){
        return out;
    } else if(out.top()){
        return in;
    } else {
        typename ProblemDomain::Values values;

        for(auto& in_value : in.values()){
            for(auto& out_value : out.values()){
                if(are_equivalent(in_value.expression, out_value.expression)){
                    values.push_back(in_value);
                }
            }
        }

        ProblemDomain result(values);
        return result;
    }
}

ProblemDomain mtac::CommonSubexpressionElimination::transfer(std::shared_ptr<mtac::BasicBlock> basic_block, mtac::Statement& statement, ProblemDomain& in){
    auto out = in;

    if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
        auto op = (*ptr)->op;
        if(mtac::erase_result(op)){
            auto it = iterate(out.values());

            while(it.has_next()){
                auto& expression = (*it).expression;

                if(expression->arg1){
                    if(auto* var_ptr = boost::get<std::shared_ptr<Variable>>(&*expression->arg1)){
                        if(*var_ptr == (*ptr)->result){
                            it.erase();
                            continue;
                        }
                    }
                }
                
                if(expression->arg2){
                    if(auto* var_ptr = boost::get<std::shared_ptr<Variable>>(&*expression->arg2)){
                        if(*var_ptr == (*ptr)->result){
                            it.erase();
                            continue;
                        }
                    }
                }

                ++it;
            }
        }

        if(mtac::is_expression((*ptr)->op)){
            bool exists = false;
            for(auto& expression : out.values()){
                if(are_equivalent(*ptr, expression.expression)){
                    exists = true;
                    break;
                }
            }

            if(!exists){
                Expression expression;
                expression.expression = *ptr;
                expression.source = basic_block;

                out.values().push_back(expression);
            }
        }
    }

    return out;
}

ProblemDomain mtac::CommonSubexpressionElimination::Boundary(std::shared_ptr<mtac::Function> /*function*/){
    return default_element();
}

ProblemDomain mtac::CommonSubexpressionElimination::Init(std::shared_ptr<mtac::Function> function){
    if(init){
        ProblemDomain result(*init);
        return result;
    }

    typename ProblemDomain::Values values;
    
    for(auto& block : function->getBasicBlocks()){
        for(auto& statement : block->statements){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
                if(mtac::is_expression((*ptr)->op)){
                    bool exists = false;
                    for(auto& expression : values){
                        if(are_equivalent(*ptr, expression.expression)){
                            exists = true;
                            break;
                        }
                    }
                    
                    if(!exists){
                        Expression expression;
                        expression.expression = *ptr;
                        expression.source = block;

                        values.push_back(expression);
                    }
                }
            }
        }
    }

    init = values;
    
    ProblemDomain result(values);
    return result;
}

bool mtac::CommonSubexpressionElimination::optimize(mtac::Statement& statement, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>>& global_results){
    auto& results = global_results->IN_S[statement];

    if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
        auto quadruple = *ptr;

        if(mtac::is_expression(quadruple->op)){
            for(auto& expression : results.values()){
                if(are_equivalent(expression.expression, quadruple)){
                    auto source_statement = expression.expression;
                    
                    mtac::Operator assign_op;
                    if(quadruple->op >= mtac::Operator::ADD && quadruple->op <= mtac::Operator::MOD){
                        assign_op = mtac::Operator::ASSIGN;
                    } else if(quadruple->op >= mtac::Operator::FADD && quadruple->op <= mtac::Operator::FDIV){
                        assign_op = mtac::Operator::FASSIGN;
                    }

                    if(optimized.find(source_statement->result) == optimized.end()){
                        std::shared_ptr<Variable> temp;
                        if(quadruple->op >= mtac::Operator::ADD && quadruple->op <= mtac::Operator::MOD){
                            temp = expression.source->context->new_temporary(INT);
                        } else if(quadruple->op >= mtac::Operator::FADD && quadruple->op <= mtac::Operator::FDIV){
                            temp = expression.source->context->new_temporary(FLOAT);
                        }

                        auto it = expression.source->statements.begin();
                        auto end = expression.source->statements.end();

                        while(it != end){
                            if(boost::get<std::shared_ptr<Quadruple>>(&*it)){
                                auto target = boost::get<std::shared_ptr<Quadruple>>(*it);
                                if(target == source_statement){
                                    source_statement->result = temp;
                                    
                                    auto quadruple = std::make_shared<mtac::Quadruple>(source_statement->result, temp, assign_op);

                                    ++it;
                                    expression.source->statements.insert(it, quadruple);

                                    break;
                                }
                            }
                        }

                        quadruple->op = assign_op;
                        quadruple->arg1 = temp;
                        quadruple->arg2.reset();
                        
                        optimized[source_statement->result] = temp;
                    }

                    quadruple->op = assign_op;
                    quadruple->arg1 = source_statement->result;
                    quadruple->arg2.reset();

                    return true;
                }
            }
        }
    }

    return false;
}
