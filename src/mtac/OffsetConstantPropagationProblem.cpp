//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "Variable.hpp"
#include "Type.hpp"
#include "VisitorUtils.hpp"
#include "FunctionContext.hpp"
#include "GlobalContext.hpp"
#include "StringPool.hpp"

#include "mtac/OffsetConstantPropagationProblem.hpp"
#include "mtac/GlobalOptimizations.hpp"
#include "mtac/LiveVariableAnalysisProblem.hpp"
#include "mtac/Quadruple.hpp"

#include "ltac/Statement.hpp"

using namespace eddic;

typedef mtac::OffsetConstantPropagationProblem::ProblemDomain ProblemDomain;

void mtac::OffsetConstantPropagationProblem::set_pool(std::shared_ptr<StringPool> string_pool){
    this->string_pool = string_pool;
}

void mtac::OffsetConstantPropagationProblem::set_platform(Platform platform){
    this->platform = platform;
}

ProblemDomain mtac::OffsetConstantPropagationProblem::Boundary(mtac::Function& function){
    pointer_escaped = mtac::escape_analysis(function);

    ProblemDomain::Values values;
    ProblemDomain out(values);
    
    for(auto& variable : function.context->stored_variables()){
        if(variable->type()->is_array() && variable->type()->has_elements()){
            auto array_size = variable->type()->elements()* variable->type()->data_type()->size(platform) + INT->size(platform);
                    
            mtac::Offset offset(variable, 0);
            out[offset] = static_cast<int>(variable->type()->elements());

            if(variable->type()->data_type() == FLOAT){
                for(std::size_t i = INT->size(platform); i < array_size; i += INT->size(platform)){
                    mtac::Offset offset(variable, i);
                    out[offset] = 0.0;
                }
            } else {
                for(std::size_t i = INT->size(platform); i < array_size; i += INT->size(platform)){
                    mtac::Offset offset(variable, i);
                    out[offset] = 0;
                }
            }
        } else if(variable->type()->is_custom_type() || variable->type()->is_template_type()){
            auto struct_size = variable->type()->size(platform);

            //All the values are set to zero
            for(std::size_t i = 0; i < struct_size; i += INT->size(platform)){
                mtac::Offset offset(variable, i);
                out[offset] = 0;
            }
            
            //Except the length of arrays that are set
            auto struct_type = function.context->global()->get_struct(variable->type()->mangle());

            while(struct_type){
                for(auto& member : struct_type->members){
                    if(member->type->is_array() && !member->type->is_dynamic_array()){
                        mtac::Offset offset(variable, function.context->global()->member_offset(struct_type, member->name));
                        out[offset] = static_cast<int>(member->type->elements());
                    }
                }

                struct_type = function.context->global()->get_struct(struct_type->parent_type);
            }
        }
    }

    return out;
}

void mtac::OffsetConstantPropagationProblem::meet(ProblemDomain& in, const ProblemDomain& out){
    mtac::intersection_meet(in, out);
}

namespace {

struct ConstantCollector : public boost::static_visitor<> {
    ProblemDomain& out;
    const mtac::Offset& offset;

    ConstantCollector(ProblemDomain& out, const mtac::Offset& offset) : out(out), offset(offset) {}

    void operator()(int value){
        out[offset] = value;
    }
    
    //Warning : Do not pass it by reference to avoid going to the template function
    void operator()(std::string value){
        out[offset] = value;
    }
    
    void operator()(double value){
        out[offset] = value;
    }
    
    void operator()(std::shared_ptr<Variable> variable){
        out[offset] = variable;
    }

    template<typename T>
    void operator()(T&){
        out.erase(offset);
    }
};

} //end of anonymous namespace

ProblemDomain mtac::OffsetConstantPropagationProblem::transfer(mtac::basic_block_p /*basic_block*/, std::shared_ptr<mtac::Quadruple>& quadruple, ProblemDomain& in){
    auto out = in;

    //Store the value assigned to result+arg1
    if(quadruple->op == mtac::Operator::DOT_ASSIGN || quadruple->op == mtac::Operator::DOT_FASSIGN || quadruple->op == mtac::Operator::DOT_PASSIGN){
        if(auto* ptr = boost::get<int>(&*quadruple->arg1)){
            if(!quadruple->result->type()->is_pointer()){
                mtac::Offset offset(quadruple->result, *ptr);

                ConstantCollector collector(out, offset);
                visit(collector, *quadruple->arg2);
            }
        } else if(boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
            auto variable = quadruple->result;

            //Impossible to know which offset is modified, consider the whole variable modified
            for(auto it = std::begin(out.values()); it != std::end(out.values());){
                auto offset = it->first;

                if(offset.variable == variable){
                    it = out.values().erase(it);
                } else {
                    ++it;
                }
            }
        }
        //PDOT Lets escape an offset
    } else if(quadruple->op == mtac::Operator::PDOT){
        if(auto* ptr = boost::get<int>(&*quadruple->arg2)){
            auto variable = boost::get<std::shared_ptr<Variable>>(*quadruple->arg1);

            mtac::Offset offset(variable, *ptr);
            escaped.insert(offset);
        }
    } else if(quadruple->op == mtac::Operator::PARAM){
        //Passing a variable by pointer erases its value
        if(quadruple->address){
            if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
                auto variable = *ptr;

                //Impossible to know if the variable is modified or not, consider it modified
                for(auto it = std::begin(out.values()); it != std::end(out.values());){
                    auto offset = it->first;

                    if(offset.variable == variable){
                        it = out.values().erase(it);
                    } else {
                        ++it;
                    }
                }
            }
        }
    }
            
    //Remove escaped variables from the result
    for(auto it = std::begin(out.values()); it != std::end(out.values());){
        auto offset = it->first;

        if(escaped.find(offset) == escaped.end()){
            ++it;
        } else {
            it = out.values().erase(it);
        }
    }

    return out;
}

bool mtac::OffsetConstantPropagationProblem::optimize(mtac::Function& function, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>> global_results){
    bool optimized = false;

    for(auto& block : function){
        for(auto& quadruple : block->statements){
            auto& results = global_results->IN_S[quadruple];

            if(results.top()){
                continue;
            }

            //If constant replace the value assigned to result by the value stored for arg1+arg2
            if(quadruple->op == mtac::Operator::DOT){
                if(auto* ptr = boost::get<int>(&*quadruple->arg2)){
                    if(auto* var_ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
                        mtac::Offset offset(*var_ptr, *ptr);

                        if(results.find(offset) != results.end() && pointer_escaped->find(offset.variable) == pointer_escaped->end()){
                            quadruple->op = mtac::Operator::ASSIGN;
                            *quadruple->arg1 = results[offset];
                            quadruple->arg2.reset();

                            if(quadruple->result->type()->is_pointer()){
                                if(auto* var_ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
                                    if(!(*var_ptr)->type()->is_pointer()){
                                        quadruple->op = mtac::Operator::PASSIGN;
                                    }
                                }
                            }

                            optimized = true;
                        }
                    } else if(auto* string_ptr = boost::get<std::string>(&*quadruple->arg1)){
                        auto string_value = string_pool->value(*string_ptr);

                        quadruple->op = mtac::Operator::ASSIGN;
                        *quadruple->arg1 = static_cast<int>(string_value[*ptr + 1]); //+1 because of the " in front of the value
                        quadruple->arg2.reset();

                        optimized = true;
                    }
                }
            } else if(quadruple->op == mtac::Operator::FDOT){
                if(auto* ptr = boost::get<int>(&*quadruple->arg2)){
                    mtac::Offset offset(boost::get<std::shared_ptr<Variable>>(*quadruple->arg1), *ptr);

                    if(results.find(offset) != results.end() && pointer_escaped->find(offset.variable) == pointer_escaped->end()){
                        quadruple->op = mtac::Operator::FASSIGN;
                        *quadruple->arg1 = results[offset];
                        quadruple->arg2.reset();

                        optimized = true;
                    }
                }
            }
        }
    }

    return optimized;
}
