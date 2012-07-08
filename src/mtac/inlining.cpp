//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <vector>

#include "SymbolTable.hpp"
#include "Options.hpp"
#include "Type.hpp"
#include "FunctionContext.hpp"

#include "mtac/inlining.hpp"

using namespace eddic;

std::size_t size_of(std::shared_ptr<mtac::Function> function){
    std::size_t size = 0;

    for(auto block : function->getBasicBlocks()){
        size += block->statements.size();
    }

    return size;
}

bool mtac::inline_functions(std::shared_ptr<mtac::Program> program){
    if(option_defined("fno-inline")){
        return false;
    }

    bool optimized = false;

    std::vector<std::shared_ptr<mtac::Function>> inlined;

    for(auto function : program->functions){
        //The main function cannot be inlined
        if(function->getName() == "main"){
            continue;
        }

        //function called once
        if(symbols.referenceCount(function->getName()) == 1){
            inlined.push_back(function); 
        } else {
            auto size = size_of(function);

            //Inline little functions
            if(size < 10){
                inlined.push_back(function);
            }
        }
    }

    for(auto source_function : inlined){
        auto source_definition = source_function->definition;
       
        for(auto dest_function : program->functions){
            if(dest_function == source_function){
                continue;
            }

            auto bit = dest_function->getBasicBlocks().begin();
            auto bend = dest_function->getBasicBlocks().end();

            while(bit != bend){
                auto basic_block = *bit;

                auto it = basic_block->statements.begin();
                auto end = basic_block->statements.end();

                while(it != end){
                    auto statement = *it;

                    if(auto* ptr = boost::get<std::shared_ptr<mtac::Call>>(&statement)){
                        auto call = *ptr;

                        if(call->functionDefinition == source_definition){
                            std::cout << "inline " << source_definition->mangledName << " in function " << dest_function->definition->mangledName << std::endl;

                            //Handle parameters
                            if(source_definition->parameters.size() > 0){
                                ++it;
                                continue;//TODO Temporary
                            }
                            
                            //Handle return type
                            if(source_definition->returnType != VOID){
                                ++it;
                                continue;//TODO Temporary
                            }

                            auto vit = source_definition->context->begin();
                            auto vend = source_definition->context->end();

                            while(vit != vend){
                                auto variable = vit->second;

                                std::cout << "extract variable " << variable->name() << std::endl;

                                ++vit;
                            }

                            auto saved_bit = bit;

                            for(auto block : source_function->getBasicBlocks()){
                                //Copy all basic blocks except ENTRY and EXIT
                                if(block->index >= 0){
                                    auto new_bb = std::make_shared<mtac::BasicBlock>(dest_function->getBasicBlocks().size() + 1);
                                    new_bb->context = block->context;
                                    new_bb->statements = block->statements;

                                    bit = dest_function->getBasicBlocks().insert(bit, new_bb);
                                    bend = dest_function->getBasicBlocks().end();
                                }
                            }

                            bit = saved_bit;

                            it = basic_block->statements.erase(it);
                            end = basic_block->statements.end();

                            continue;
                        }
                    }

                    ++it;
                }

                ++bit;
            }
        }
    }

    return optimized;
}
