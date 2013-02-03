//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/range/adaptors.hpp>

#include "Type.hpp"
#include "FunctionContext.hpp"
#include "GlobalContext.hpp"
#include "Variable.hpp"

#include "mtac/DeadCodeElimination.hpp"
#include "mtac/GlobalOptimizations.hpp"
#include "mtac/LiveVariableAnalysisProblem.hpp"
#include "mtac/Utils.hpp"
#include "mtac/Offset.hpp"
#include "mtac/Quadruple.hpp"

using namespace eddic;

bool mtac::dead_code_elimination::operator()(mtac::Function& function){
    bool optimized = false;

    mtac::LiveVariableAnalysisProblem problem;
    auto results = mtac::data_flow(function, problem);

    std::vector<std::size_t> to_delete;

    for(auto& block : function){
        auto& out = results->OUT[block];

        for(auto& quadruple : boost::adaptors::reverse(block->statements)){
            if(quadruple.result && mtac::erase_result(quadruple.op)){
                if(out.top() || out.values().find(quadruple.result) == out.values().end()){
                    to_delete.push_back(quadruple.uid());
                }
            }

            problem.transfer(block, quadruple, out);
        }
    }

    if(!to_delete.empty()){
        optimized = true;

        for(auto& block : function){
            block->statements.erase(
                    remove_if(block->statements.begin(), block->statements.end(), 
                        [&to_delete](const mtac::Quadruple& q){return std::find(to_delete.begin(), to_delete.end(), q.uid()) != to_delete.end();}), 
                    block->statements.end());
        }
    }

    //TODO Review or remove this optimization, this is quite unsafe

    std::unordered_set<Offset, mtac::OffsetHash> used_offsets;
    std::unordered_set<std::shared_ptr<Variable>> invalidated_offsets;

    for(auto& block : function){
        for(auto& quadruple : block->statements){
            if(quadruple.op == mtac::Operator::DOT || quadruple.op == mtac::Operator::FDOT || quadruple.op == mtac::Operator::PDOT){
                if(auto* var_ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple.arg1)){
                    if(auto* offset_ptr = boost::get<int>(&*quadruple.arg2)){
                        mtac::Offset offset(*var_ptr, *offset_ptr);
                        used_offsets.insert(offset);
                    } else {
                        invalidated_offsets.insert(*var_ptr);
                    }
                }
            }
        }
    }

    for(auto& block : function){
        auto it = iterate(block->statements);

        while(it.has_next()){
            auto& quadruple = *it;

            if(quadruple.op == mtac::Operator::DOT_ASSIGN || quadruple.op == mtac::Operator::DOT_FASSIGN || quadruple.op == mtac::Operator::DOT_PASSIGN){
                if(invalidated_offsets.find(quadruple.result) == invalidated_offsets.end()){
                    //Arrays are a problem because they are not considered as escaped after being passed in parameters
                    if(!quadruple.result->type()->is_pointer() && !quadruple.result->type()->is_array()){
                        if(auto* offset_ptr = boost::get<int>(&*quadruple.arg1)){
                            if(quadruple.result->type()->is_custom_type() || quadruple.result->type()->is_template_type()){
                                auto struct_type = function.context->global()->get_struct(quadruple.result->type()->mangle());
                                auto member_type = function.context->global()->member_type(struct_type, *offset_ptr);

                                if(member_type->is_pointer()){
                                    ++it;
                                    continue;
                                }
                            }

                            mtac::Offset offset(quadruple.result, *offset_ptr);

                            if(problem.pointer_escaped->find(quadruple.result) == problem.pointer_escaped->end() && used_offsets.find(offset) == used_offsets.end()){
                                it.erase();
                                optimized=true;
                                continue;
                            }
                        }
                    }
                }
            }

            ++it;
        }
    }

    return optimized;
}
