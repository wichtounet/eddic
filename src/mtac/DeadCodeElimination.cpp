//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <list>

#include "Type.hpp"
#include "FunctionContext.hpp"
#include "GlobalContext.hpp"
#include "Variable.hpp"

#include "mtac/DeadCodeElimination.hpp"
#include "mtac/GlobalOptimizations.hpp"
#include "mtac/LiveVariableAnalysisProblem.hpp"
#include "mtac/Utils.hpp"
#include "mtac/Offset.hpp"
#include "mtac/Statement.hpp"

#include "ltac/Statement.hpp"

using namespace eddic;

bool mtac::dead_code_elimination::operator()(std::shared_ptr<mtac::Function> function){
    bool optimized = false;

    mtac::LiveVariableAnalysisProblem problem;
    auto results = mtac::data_flow(function, problem);

    for(auto& block : function){
        auto it = iterate(block->statements);

        while(it.has_next()){
            auto statement = *it;

            if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
                if(mtac::erase_result((*ptr)->op)){
                    if(results->OUT_S[statement].values().find((*ptr)->result) == results->OUT_S[statement].values().end()){
                        it.erase();
                        optimized=true;
                        continue;
                    }
                }
            }

            ++it;
        }
    }

    std::unordered_set<Offset, mtac::OffsetHash> used_offsets;

    for(auto& block : function){
        for(auto& statement : block->statements){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
                auto quadruple = *ptr;
                
                if(quadruple->op == mtac::Operator::DOT || quadruple->op == mtac::Operator::FDOT || quadruple->op == mtac::Operator::PDOT){
                    if(auto* var_ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
                        if(auto* offset_ptr = boost::get<int>(&*quadruple->arg2)){
                            mtac::Offset offset(*var_ptr, *offset_ptr);
                            used_offsets.insert(offset);
                        }
                    }
                }
            }
        }
    }
    
    for(auto& block : function){
        auto it = block->statements.begin();
        auto end = block->statements.end();

        while(it != end){
            auto statement = *it;
            
            if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
                auto quadruple = *ptr;
                
                if(quadruple->op == mtac::Operator::DOT_ASSIGN || quadruple->op == mtac::Operator::DOT_FASSIGN || quadruple->op == mtac::Operator::DOT_PASSIGN){
                    //Arrays are a problem because they are not considered as escaped after being passed in parameters
                    if(!quadruple->result->type()->is_pointer() && !quadruple->result->type()->is_array()){
                        if(auto* offset_ptr = boost::get<int>(&*quadruple->arg1)){
                            if(quadruple->result->type()->is_custom_type() || quadruple->result->type()->is_template()){
                                auto struct_type = function->context->global()->get_struct(quadruple->result->type()->mangle());
                                auto member_type = function->context->global()->member_type(struct_type, *offset_ptr);
                                
                                if(member_type->is_pointer()){
                                    ++it;
                                    continue;
                                }
                            }

                            mtac::Offset offset(quadruple->result, *offset_ptr);

                            if(problem.pointer_escaped->find(quadruple->result) == problem.pointer_escaped->end() && used_offsets.find(offset) == used_offsets.end()){
                                it = block->statements.erase(it);
                                end = block->statements.end();
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
