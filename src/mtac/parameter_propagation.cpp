//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <unordered_map>
#include <vector>

#include "Function.hpp"
#include "GlobalContext.hpp"

#include "mtac/parameter_propagation.hpp"
#include "mtac/Program.hpp"
#include "mtac/Function.hpp"
#include "mtac/Call.hpp"
#include "mtac/Argument.hpp"
#include "mtac/Param.hpp"

using namespace eddic;

namespace {

typedef std::unordered_map<std::string, std::vector<std::vector<mtac::Argument>>> Arguments;

Arguments collect_arguments(mtac::Program& program){
    Arguments arguments;

    for(auto& function : program.functions){
        for(auto& block : function){
            for(auto& statement : block){
                if(auto* ptr = boost::get<std::shared_ptr<mtac::Call>>(&statement)){
                    auto& function = (*ptr)->functionDefinition;

                    if(!function.standard && !function.parameters().empty()){
                        std::vector<mtac::Argument> function_arguments;

                        auto parameters = function.parameters().size();
                        auto param_block = block->prev;

                        auto it = param_block->statements.rbegin();
                        auto end = param_block->statements.rend();

                        auto discovered = 0;

                        while(it != end && discovered < parameters){
                            auto& param_statement = *it;

                            if(auto* param_ptr = boost::get<std::shared_ptr<mtac::Param>>(&param_statement)){
                                ++discovered;

                                function_arguments.push_back((*param_ptr)->arg);
                            }

                            ++it;
                        }
                        
                        arguments[function.mangledName].push_back(std::move(function_arguments));
                    }
                }
            }
        }
    }
    
    return arguments;
}

} //end of anonymous namespace 

bool mtac::parameter_propagation::operator()(mtac::Program& program){
    bool optimized = false;

    auto global_context = program.context;

    auto arguments = collect_arguments(program);

    for(auto& function_map : arguments){
        auto& function_name = function_map.first;
        auto& function = global_context->getFunction(function_name);
        auto& function_arguments = function_map.second;

        for(int i = 0; i < function.parameters().size(); ++i){
            bool found = false;
            int constant_value = 0;

            for(auto& arguments : function_arguments){
                auto& arg = arguments[i];

                if(auto* ptr = boost::get<int>(&arg)){
                    if(found){
                        if(*ptr != constant_value){
                            found = false;
                            break;
                        }
                    } else {
                        found = true;
                        constant_value = *ptr;
                    }
                } else {
                    found = false;
                    break;
                }
            }

            if(found){
                //TODO Remove parameter i from function
            }
        }
    }

    return optimized;
}
