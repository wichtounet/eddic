//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "Variable.hpp"
#include "FunctionContext.hpp"

#include "mtac/CommonSubexpressionElimination.hpp"

using namespace eddic;

typedef mtac::CommonSubexpressionElimination::ProblemDomain ProblemDomain;

std::ostream& mtac::operator<<(std::ostream& stream, Expression& expression){
    return stream << "Expression {expression = {}}";
}

bool is_distributive(mtac::Operator op){
    return op == mtac::Operator::ADD || op == mtac::Operator::FADD || op == mtac::Operator::MUL || op == mtac::Operator::FMUL;
}

bool are_equivalent(std::shared_ptr<mtac::Quadruple> first, std::shared_ptr<mtac::Quadruple> second){
    if(first->op != second->op){
        return false;
    }

    if(is_distributive(first->op)){
        return (*first->arg1 == *second->arg1 && *first->arg2 == *second->arg2) || (*first->arg1 == *second->arg2 && *first->arg2 == *second->arg1);
    } else {
        return (*first->arg1 == *second->arg1 && *first->arg2 == *second->arg2);
    }
}

bool is_boundary(ProblemDomain& out){
   return !out.top() && out.values().size() == 1 && !out.values()[0].source; 
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

bool is_expression(std::shared_ptr<mtac::Quadruple> quadruple){
    return quadruple->op >= mtac::Operator::ADD && quadruple->op <= mtac::Operator::FDIV;
}

ProblemDomain mtac::CommonSubexpressionElimination::transfer(std::shared_ptr<mtac::BasicBlock> basic_block, mtac::Statement& statement, ProblemDomain& in){
    auto out = in;

    if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
        auto op = (*ptr)->op;
        if(op != mtac::Operator::ARRAY_ASSIGN && op != mtac::Operator::DOT_ASSIGN && op != mtac::Operator::RETURN){
            std::vector<unsigned int> killed;
            
            for(unsigned int i = 0; i < in.values().size(); ++i){
                auto& expression = in.values()[i].expression;

                auto old_size = killed.size();

                if(expression->arg1 && boost::get<std::shared_ptr<Variable>>(&*expression->arg1)){
                    auto var = boost::get<std::shared_ptr<Variable>>(*expression->arg1);

                    if(var == (*ptr)->result){
                        killed.push_back(i);
                    }
                }
                
                if(old_size == killed.size() && expression->arg2 && boost::get<std::shared_ptr<Variable>>(&*expression->arg2)){
                    auto var = boost::get<std::shared_ptr<Variable>>(*expression->arg2);

                    if(var == (*ptr)->result){
                        killed.push_back(i);
                    }
                }
            }

            std::reverse(killed.begin(), killed.end());

            for(auto i : killed){
                out.values().erase(out.values().begin() + i);
            }
        }

        if(is_expression(*ptr)){
            bool exists = false;
            for(auto& expression : out.values()){
                if(are_equivalent(*ptr, expression.expression)){
                    exists = true;
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

ProblemDomain mtac::CommonSubexpressionElimination::Boundary(std::shared_ptr<mtac::Function> function){
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
                if(is_expression(*ptr)){
                    bool exists = false;
                    for(auto& expression : values){
                        if(are_equivalent(*ptr, expression.expression)){
                            exists = true;
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

    bool changes = false;

    if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
        if(is_expression(*ptr)){
            for(auto& expression : results.values()){
                if(are_equivalent(expression.expression, *ptr)){
                    auto source_statement = expression.expression;

                    mtac::Operator assign_op;
                    if((*ptr)->op >= mtac::Operator::ADD && (*ptr)->op <= mtac::Operator::MOD){
                        assign_op = mtac::Operator::ASSIGN;
                    } else if((*ptr)->op >= mtac::Operator::FADD && (*ptr)->op <= mtac::Operator::FDIV){
                        assign_op = mtac::Operator::FASSIGN;
                    }

                    if(source_statement->result->position().isTemporary()){
                        (*ptr)->op = assign_op;
                        (*ptr)->arg1 = source_statement->result;
                    } else {
                        std::shared_ptr<Variable> temp;
                        if((*ptr)->op >= mtac::Operator::ADD && (*ptr)->op <= mtac::Operator::MOD){
                            temp = expression.source->context->new_temporary(INT);
                        } else if((*ptr)->op >= mtac::Operator::FADD && (*ptr)->op <= mtac::Operator::FDIV){
                            temp = expression.source->context->new_temporary(FLOAT);
                        }

                        auto it = expression.source->statements.begin();
                        auto end = expression.source->statements.end();

                        while(it != end){
                            if(boost::get<std::shared_ptr<Quadruple>>(&*it)){
                                auto target = boost::get<std::shared_ptr<Quadruple>>(*it);
                                if(target == source_statement){
                                    auto quadruple = std::make_shared<mtac::Quadruple>(source_statement->result, temp, assign_op);

                                    it = expression.source->statements.insert(it, quadruple);
                                }
                            }

                            ++it;
                        }

                        source_statement->result = temp;

                        (*ptr)->op = assign_op;
                        (*ptr)->arg1 = temp;
                    }

                    changes = true;

                    break;
                }
            }
        }
    }

    return changes;
}
