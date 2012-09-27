//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ast/PassManager.hpp"
#include "ast/Pass.hpp"
#include "ast/SourceFile.hpp"
#include "ast/TemplateEngine.hpp"

//The passes
#include "ast/TransformerEngine.hpp"
#include "ast/TemplateCollectionPass.hpp"

using namespace eddic;

namespace {
    
void apply_pass(std::shared_ptr<ast::Pass> pass, ast::SourceFile& program){
    pass->apply_program(program);

    for(auto& block : program.Content->blocks){
        if(auto* ptr = boost::get<ast::FunctionDeclaration>(&block)){
            pass->apply_function(*ptr);
        } else if(auto* ptr = boost::get<ast::Struct>(&block)){
            pass->apply_struct(*ptr, false);

            for(auto& function : ptr->Content->functions){
                pass->apply_struct_function(function);
            }

            for(auto& function : ptr->Content->destructors){
                pass->apply_struct_destructor(function);
            }

            for(auto& function : ptr->Content->constructors){
                pass->apply_struct_constructor(function);
            }
        }
    }
}

void handle_instantiations(ast::TemplateEngine& template_engine, std::vector<std::shared_ptr<ast::Pass>>& applied_passes, ast::SourceFile& program){
    for(auto& context_pair : template_engine.function_template_instantiated){
        auto context = context_pair.first;
        auto instantiated_functions = context_pair.second;

        if(context.empty()){
            for(auto& function : instantiated_functions){
                for(auto& pass : applied_passes){
                    pass->apply_function(function);
                }
            }
        } else {
            for(auto& block : program.Content->blocks){
                if(auto* struct_type = boost::get<ast::Struct>(&block)){
                    if(struct_type->Content->struct_type->mangle() == context){
                        for(auto& pass : applied_passes){
                            pass->apply_struct(*struct_type, true);

                            for(auto& function : instantiated_functions){
                                pass->apply_struct_function(function);
                            }
                        }

                        break;
                    }
                }
            }
        }
    }
}

void add_instantiations_to_program(ast::TemplateEngine& template_engine, ast::SourceFile& program){
    for(auto& struct_ : template_engine.class_template_instantiated){
        program.Content->blocks.push_back(struct_);
    }

    for(auto& context_pair : template_engine.function_template_instantiated){
        auto context = context_pair.first;
        auto instantiated_functions = context_pair.second;

        if(context.empty()){
            for(auto& function : instantiated_functions){
                program.Content->blocks.push_back(function);
            }
        } else {
            for(auto& block : program.Content->blocks){
                if(auto* struct_type = boost::get<ast::Struct>(&block)){
                    if(struct_type->Content->struct_type->mangle() == context){
                        for(auto& function : instantiated_functions){
                            struct_type->Content->functions.push_back(function);
                        }

                        break;
                    }
                }
            }
        }
    }

    //They are no more instantiated
    template_engine.class_template_instantiated.clear();
    template_engine.function_template_instantiated.clear();
}

} //end of anonymous namespace

ast::PassManager::PassManager(ast::TemplateEngine& template_engine) : template_engine(template_engine) {
    //NOP
}

void ast::PassManager::init_passes(){
    //Clean pass
    passes.push_back(std::make_shared<ast::CleanPass>(template_engine));

    //Template Collection pass
    passes.push_back(std::make_shared<ast::TemplateCollectionPass>(template_engine));
}

void ast::PassManager::run_passes(ast::SourceFile& program){
    for(auto& pass : passes){
        //A simple pass is only applied once to the whole program
        if(pass->is_simple()){
            //It is up to the simple pass to recurse into the program
            pass->apply_program(program);
        } 
        //Normal pass are applied until all function and structures have been handled
        else {
            //The next passes will have to apply it again to fresh functions
            applied_passes.push_back(pass);

            apply_pass(pass, program);

            for(auto& struct_ : template_engine.class_template_instantiated){
                for(auto& pass : applied_passes){
                    pass->apply_struct(struct_, false);
                }
            }

            handle_instantiations(template_engine, applied_passes, program);

            add_instantiations_to_program(template_engine, program);
        }
    }
}
