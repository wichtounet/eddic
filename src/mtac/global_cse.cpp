//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <algorithm>

#include "assert.hpp"
#include "Variable.hpp"
#include "FunctionContext.hpp"
#include "Type.hpp"
#include "iterators.hpp"
#include "GlobalContext.hpp"

#include "mtac/global_cse.hpp"
#include "mtac/Utils.hpp"
#include "mtac/Quadruple.hpp"

using namespace eddic;

typedef mtac::global_cse::ProblemDomain ProblemDomain;

void mtac::global_cse::meet(ProblemDomain& in, const ProblemDomain& out){
    eddic_assert(!in.top() || !out.top(), "At least one lattice should not be a top element");

    if(in.top()){
        in = out;
    } else if(out.top()){
        //in does not change
    } else {
        auto& first = in.values();
        auto& second = out.values();
        
        std::set<expression> intersection;

        std::set_intersection(first.begin(), first.end(), second.begin(), second.end(), std::inserter(intersection, intersection.begin()));

        in.values() = std::move(intersection);
    }
}

ProblemDomain mtac::global_cse::Boundary(mtac::Function& function){
    this->function = &function;
    
    pointer_escaped = mtac::escape_analysis(function);

    typename ProblemDomain::Values values;
    
    //Compute Eval(i)

    for(auto& block : function){
        for(auto& q : block->statements){
            if(mtac::is_expression(q.op) && mtac::is_valid(q, pointer_escaped) && mtac::is_interesting(q)){
                Eval[block].insert({0, *q.arg1, *q.arg2, q.op, nullptr, q.result->type()});
            }

            mtac::kill_expressions(q, Eval[block]);
        }
    }

    //Compute Kill(i)
    
    for(auto& block : function){
        for(auto& q : block->statements){
            auto op = q.op;
            if(mtac::erase_result(op) || op == mtac::Operator::DOT_ASSIGN || op == mtac::Operator::DOT_FASSIGN || op == mtac::Operator::DOT_PASSIGN){
                for(auto& b : function){
                    if(b != block){
                        for(auto& expression : Eval[b]){
                            if(mtac::is_killing(q, expression)){
                                Kill[block].insert(expression);
                            }
                        }
                    }
                }
            }
        }
    }

    Expressions expressions;

    //Compute Uexp

    for(auto& block : function){
        for(auto& expression : Eval[block]){
            expressions.insert(expression);
        }
    }

    init = std::move(expressions);

    return ProblemDomain(ProblemDomain::Values());
}

ProblemDomain mtac::global_cse::Init(mtac::Function&){
    ProblemDomain result(*init);
    return result;
}

void mtac::global_cse::transfer(mtac::basic_block_p basic_block, ProblemDomain& out){
    auto& out_values = out.values();
    auto it = out_values.begin();

    //Compute AEin - Kill(i)

    while(it != out_values.end()){
        if(Kill[basic_block].find(*it) != Kill[basic_block].end()){
            it = out_values.erase(it);
            continue;
        }

        ++it;
    }

    //Compute Eval(i) U (AEin - Kill(i))

    for(auto& expression : Eval[basic_block]){
        out_values.insert(expression);
    }
}

namespace {

void search_path(const mtac::expression& exp, std::shared_ptr<Variable>& tj, mtac::Operator op, mtac::basic_block_p& block, std::unordered_set<mtac::basic_block_p>& visited){
    if(visited.find(block) != visited.end()){
        return;
    }

    visited.insert(block);
    
    if(!block->statements.empty()){
        auto it = block->end();
        auto end = block->begin();

        do {
            --it;

            auto& quadruple = *it;
            if(mtac::is_expression(quadruple.op) && mtac::are_equivalent(quadruple, exp)){
                quadruple.op = op;
                quadruple.arg1 = tj;
                quadruple.arg2.reset();

                block->statements.insert(it, mtac::Quadruple(tj, exp.arg1, exp.op, exp.arg2));

                return;
            }
        } while(it != end);
    }

    eddic_assert(!block->predecessors.empty(), "There must be an equivalent expression on each backward path");

    for(auto& P : block->predecessors){
        search_path(exp, tj, op, P, visited);
    }
}

} //end of anonymous namespace

bool mtac::global_cse::optimize(mtac::Function& function, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>> global_results){
    bool changes = false;

    for(auto& i : function){
        if(global_results->IN[i].top()){
            continue;
        }

        auto& AEin = global_results->IN[i].values();

        for(auto& exp : Eval[i]){
            if(AEin.find(exp) != AEin.end()){
                function.context->global()->stats().inc_counter("common_subexpr_eliminated");

                changes = true;

                auto it = i->begin();

                while(!mtac::are_equivalent(*it, exp) && it != i->end()){
                    ++it;
                }

                eddic_assert(it != i->end(), "The expression must be found because it is in Eval(i)");

                auto& quadruple = *it;

                if(it != i->begin()){
                    bool global_cs = true;

                    do {
                        --it;

                        if(mtac::erase_result(it->op) || it->op == mtac::Operator::DOT_ASSIGN || it->op == mtac::Operator::DOT_FASSIGN || it->op == mtac::Operator::DOT_PASSIGN){
                            if(mtac::is_killing(*it, exp)){
                                global_cs = false;
                                break;
                            }
                        }
                    } while(it != i->begin());

                    if(!global_cs){
                        continue;
                    }
                }

                auto tj = function.context->new_temporary(exp.type);
                mtac::Operator op = mtac::assign_op(exp.op);

                quadruple.op = op;
                quadruple.arg1 = tj;
                quadruple.arg2.reset();

                std::unordered_set<mtac::basic_block_p> visited;
                visited.insert(i);

                for(auto& P : i->predecessors){
                    search_path(exp, tj, op, P, visited);
                }
            }
        }
    }

    return changes;
}

bool mtac::operator==(const mtac::Domain<Expressions>& lhs, const mtac::Domain<Expressions>& rhs){
    if(lhs.top() || rhs.top()){
        return lhs.top() == rhs.top();
    }

    auto& lhs_values = lhs.values();
    auto& rhs_values = rhs.values();

    if(lhs_values.size() != rhs_values.size()){
        return false;
    }

    for(auto& lhs_expression : lhs_values){
        if(rhs_values.find(lhs_expression) == rhs_values.end()){
            return false;
        }
    }

    return true;
}

bool mtac::operator!=(const mtac::Domain<Expressions>& lhs, const mtac::Domain<Expressions>& rhs){
    return !(lhs == rhs);
}
