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
#include "ast/ContextAnnotator.hpp"
#include "ast/StructuresAnnotator.hpp"
#include "ast/DefaultValues.hpp"
#include "ast/FunctionsAnnotator.hpp"
#include "ast/VariablesAnnotator.hpp"
#include "ast/FunctionsAnnotator.hpp"

using namespace eddic;

/*
 * TODO Check if the overhead of copying each block of the program and each 
 * block of each struct is not too high. Another solution would be to store temporarily
 * all the instantiated class and function templates and add then after the pass for the next pass. 
*/

namespace {
    
void apply_pass(std::shared_ptr<ast::Pass> pass, ast::SourceFile& program){
    for(unsigned int i = 0; i < pass->passes(); ++i){
        pass->set_current_pass(i);
        pass->apply_program(program, false);

        std::vector<ast::FirstLevelBlock> blocks = program.Content->blocks;
        for(auto& block : blocks){
            if(auto* ptr = boost::get<ast::FunctionDeclaration>(&block)){
                pass->apply_function(*ptr);
            } else if(auto* ptr = boost::get<ast::Struct>(&block)){
                pass->apply_struct(*ptr, false);

                std::vector<ast::FunctionDeclaration> functions = ptr->Content->functions;
                for(auto& function : ptr->Content->functions){
                    pass->apply_struct_function(function);
                }

                std::vector<ast::Destructor> destructors = ptr->Content->destructors;
                for(auto& function : destructors){
                    pass->apply_struct_destructor(function);
                }

                std::vector<ast::Constructor> constructors = ptr->Content->constructors;
                for(auto& function : constructors){
                    pass->apply_struct_constructor(function);
                }
            }
        }
    }
}

template<typename Pass>
std::shared_ptr<Pass> make_pass(std::shared_ptr<ast::TemplateEngine> template_engine, Platform platform, std::shared_ptr<Configuration> configuration){
    auto pass = std::make_shared<Pass>();
    
    pass->set_template_engine(template_engine);
    pass->set_platform(platform);
    pass->set_configuration(configuration);

    return pass;
}

} //end of anonymous namespace

ast::PassManager::PassManager(Platform platform, std::shared_ptr<Configuration> configuration) : platform(platform), configuration(configuration) {
    template_engine = std::make_shared<ast::TemplateEngine>(*this);
}

void ast::PassManager::init_passes(){
    //Clean pass
    passes.push_back(make_pass<ast::CleanPass>(template_engine, platform, configuration));
    
    //Context annotation pass
    passes.push_back(make_pass<ast::ContextAnnotationPass>(template_engine, platform, configuration));
    
    //Structures collection pass
    passes.push_back(make_pass<ast::StructureCollectionPass>(template_engine, platform, configuration));

    //Template Collection pass
    passes.push_back(make_pass<ast::TemplateCollectionPass>(template_engine, platform, configuration));
    
    //Structures member collection pass
    passes.push_back(make_pass<ast::StructureMemberCollectionPass>(template_engine, platform, configuration));
    
    //Structures check pass
    passes.push_back(make_pass<ast::StructureCheckPass>(template_engine, platform, configuration));
    
    //Structures check pass
    passes.push_back(make_pass<ast::DefaultValuesPass>(template_engine, platform, configuration));
    
    //Member function collection pass
    passes.push_back(make_pass<ast::MemberFunctionCollectionPass>(template_engine, platform, configuration));

    //Variables annotation pass
    passes.push_back(make_pass<ast::VariableAnnotationPass>(template_engine, platform, configuration));
    
    //Function collection pass
    passes.push_back(make_pass<ast::FunctionCollectionPass>(template_engine, platform, configuration));
    
    //Function check pass
    passes.push_back(make_pass<ast::FunctionCheckPass>(template_engine, platform, configuration));
}
        
void ast::PassManager::function_instantiated(ast::FunctionDeclaration& function, const std::string& context){
    for(auto& pass : applied_passes){
        for(unsigned int i = 0; i < pass->passes(); ++i){
            pass->set_current_pass(i);
            pass->apply_program(program, true);

            if(context.empty()){
                pass->apply_function(function);
            } else {
                for(auto& block : program.Content->blocks){
                    if(auto* struct_type = boost::get<ast::Struct>(&block)){
                        if(struct_type->Content->struct_type->mangle() == context){
                            pass->apply_struct(*struct_type, true);
                            pass->apply_struct_function(function);
                        }

                        break;
                    }
                }
            }
        }
    }
    
    if(context.empty()){
        program.Content->blocks.push_back(function);
    } else {
        for(auto& block : program.Content->blocks){
            if(auto* struct_type = boost::get<ast::Struct>(&block)){
                if(struct_type->Content->struct_type->mangle() == context){
                    struct_type->Content->functions.push_back(function);
                    break;
                }
            }
        }
    }
}

void ast::PassManager::struct_instantiated(ast::Struct& struct_){
    for(auto& pass : applied_passes){
        for(unsigned int i = 0; i < pass->passes(); ++i){
            pass->set_current_pass(i);
            pass->apply_program(program, true);
            pass->apply_struct(struct_, false);
        }
    }
    
    program.Content->blocks.push_back(struct_);
}

void ast::PassManager::run_passes(ast::SourceFile& program){
    this->program = program;

    for(auto& pass : passes){
        //A simple pass is only applied once to the whole program
        if(pass->is_simple()){
            for(unsigned int i = 0; i < pass->passes(); ++i){
                pass->set_current_pass(i);
                
                //It is up to the simple pass to recurse into the program
                pass->apply_program(program, false);
            }
        } 
        //Normal pass are applied until all function and structures have been handled
        else {
            //The next passes will have to apply it again to fresh functions
            applied_passes.push_back(pass);

            apply_pass(pass, program);
        }
    }
}
