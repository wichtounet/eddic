//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#define BOOST_NO_RTTI
#define BOOST_NO_TYPEID
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

    std::vector<std::size_t> to_delete;

    //1. DCE based on data-flow analysis

    mtac::LiveVariableAnalysisProblem problem;
    auto results = mtac::data_flow(function, problem);

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

    //Delete what has been found on previous steps

    if(!to_delete.empty()){
        optimized = true;

        for(auto& block : function){
            for(auto& quadruple : block->statements){
                if(std::find(to_delete.begin(), to_delete.end(), quadruple.uid()) != to_delete.end()){
                    mtac::transform_to_nop(quadruple);
                }
            }
        }
    }

    //2. Remove variables that contribute only to themselves
    //TODO This could probably be done directly in the data-flow DCE

    std::unordered_set<std::shared_ptr<Variable>> candidates;

    for(auto& block : function){
        for(auto& quadruple : block->statements){
            if(quadruple.result && mtac::erase_result(quadruple.op)){
                if_init_equals(quadruple.arg1, quadruple.result, [&candidates, &quadruple](){ candidates.insert(quadruple.result);});
                if_init_equals(quadruple.arg2, quadruple.result, [&candidates, &quadruple](){ candidates.insert(quadruple.result);});
            }
        }
    }
    
    for(auto& block : function){
        for(auto& quadruple : block->statements){
            if(quadruple.result && mtac::erase_result(quadruple.op)){
                if_init_not_equals<std::shared_ptr<Variable>>(quadruple.arg1, quadruple.result, [&candidates](std::shared_ptr<Variable>& var){ candidates.erase(var);});
                if_init_not_equals<std::shared_ptr<Variable>>(quadruple.arg2, quadruple.result, [&candidates](std::shared_ptr<Variable>& var){ candidates.erase(var);});
            } else {
                candidates.erase(quadruple.result);

                if_init<std::shared_ptr<Variable>>(quadruple.arg1, [&candidates](std::shared_ptr<Variable>& var){ candidates.erase(var); });
                if_init<std::shared_ptr<Variable>>(quadruple.arg2, [&candidates](std::shared_ptr<Variable>& var){ candidates.erase(var); });
            }
        }
    }

    if(!candidates.empty()){
        for(auto& block : function){
            for(auto& quadruple : block->statements){
                if(quadruple.result && mtac::erase_result(quadruple.op) && candidates.find(quadruple.result) != candidates.end()){
                    mtac::transform_to_nop(quadruple);
                    optimized = true;
                } 
            }
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
        for(auto& quadruple : block->statements){
            if(quadruple.op == mtac::Operator::DOT_ASSIGN || quadruple.op == mtac::Operator::DOT_FASSIGN || quadruple.op == mtac::Operator::DOT_PASSIGN){
                if(invalidated_offsets.find(quadruple.result) == invalidated_offsets.end()){
                    //Arrays are a problem because they are not considered as escaped after being passed in parameters
                    if(!quadruple.result->type()->is_pointer() && !quadruple.result->type()->is_array()){
                        if(auto* offset_ptr = boost::get<int>(&*quadruple.arg1)){
                            if(quadruple.result->type()->is_custom_type() || quadruple.result->type()->is_template_type()){
                                auto struct_type = function.context->global()->get_struct(quadruple.result->type()->mangle());
                                auto member_type = function.context->global()->member_type(struct_type, *offset_ptr);

                                if(member_type->is_pointer()){
                                    continue;
                                }
                            }

                            mtac::Offset offset(quadruple.result, *offset_ptr);

                            if(problem.pointer_escaped->find(quadruple.result) == problem.pointer_escaped->end() && used_offsets.find(offset) == used_offsets.end()){
                                mtac::transform_to_nop(quadruple);
                                optimized=true;
                            }
                        }
                    }
                }
            }
        }
    }

    return optimized;
}
