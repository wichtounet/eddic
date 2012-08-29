//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "EDDIFrontEnd.hpp"
#include "SemanticalException.hpp"
#include "DebugStopWatch.hpp"
#include "Options.hpp"
#include "StringPool.hpp"
#include "Type.hpp"
#include "GlobalContext.hpp"

#include "parser/SpiritParser.hpp"

#include "ast/SourceFile.hpp"

//Annotators
#include "ast/DefaultValues.hpp"
#include "ast/ContextAnnotator.hpp"
#include "ast/FunctionsAnnotator.hpp"
#include "ast/VariablesAnnotator.hpp"
#include "ast/StructuresAnnotator.hpp"

//Checkers
#include "ast/StringChecker.hpp"
#include "ast/TypeChecker.hpp"

//Visitors
#include "ast/DependenciesResolver.hpp"
#include "ast/OptimizationEngine.hpp"
#include "ast/TransformerEngine.hpp"
#include "ast/WarningsEngine.hpp"
#include "ast/Printer.hpp"
#include "ast/TemplateEngine.hpp"

#include "mtac/Compiler.hpp"
#include "mtac/RegisterAllocation.hpp"

using namespace eddic;

void check_for_main(std::shared_ptr<GlobalContext> context);
void mark_functions(ast::SourceFile& program);
bool still_unmarked_functions(ast::SourceFile& program);

std::shared_ptr<mtac::Program> EDDIFrontEnd::compile(const std::string& file){
    parser::SpiritParser parser;

    //The program to build
    ast::SourceFile program;

    //Parse the file into the program
    bool parsing = parser.parse(file, program); 

    //If the parsing was sucessfully
    if(parsing){
        set_string_pool(std::make_shared<StringPool>());

        //Read dependencies
        resolveDependencies(program, parser);

        //Apply some cleaning transformations
        ast::cleanAST(program);

        ast::TemplateEngine template_engine;

        do {
            //Define contexts and structures
            ast::defineContexts(program);
            ast::defineStructures(program);

            //Add default values
            ast::defineDefaultValues(program);

            //Fill the string pool
            ast::checkStrings(program, *pool);

            //Add some more informations to the AST
            ast::defineMemberFunctions(program);
            ast::defineVariables(program);
            ast::defineFunctions(program);

            //Static analysis
            ast::checkTypes(program);

            //Transform the AST
            ast::transformAST(program);

            //Mark all the functions as transformed
            mark_functions(program);
            
            //Instantiate templates
            template_engine.template_instantiation(program);
        } while(still_unmarked_functions(program));

        //Check for warnings
        ast::checkForWarnings(program);

        //Check that there is a main in the program
        check_for_main(program.Content->context);

        //Optimize the AST
        ast::optimizeAST(program, *pool);

        //If the user asked for it, print the Abstract Syntax Tree
        if(option_defined("ast") || option_defined("ast-only")){
            ast::Printer printer;
            printer.print(program);
        }
        
        //If the user wants only the AST prints, it is not necessary to compile the AST
        if(option_defined("ast-only")){
            return nullptr;
        }

        std::shared_ptr<mtac::Program> mtacProgram = std::make_shared<mtac::Program>();

        //Generate Three-Address-Code language
        mtac::Compiler compiler;
        compiler.compile(program, pool, mtacProgram);

        return mtacProgram;
    }

    //If the parsing fails, the error is already printed to the console
    return nullptr;
}

void check_for_main(std::shared_ptr<GlobalContext> context){
    if(context->exists("_F4main")){
        context->addReference("_F4main");
    } else if (context->exists("_F4mainAS")){
        context->addReference("_F4mainAS");
    } else {
        throw SemanticalException("The program does not contain a valid main function"); 
    }
}

void mark_functions(ast::SourceFile& program){
    for(auto& block : program.Content->blocks){
        if(auto* ptr = boost::get<ast::FunctionDeclaration>(&block)){
            ptr->Content->marked = true;
        } else if(auto* ptr = boost::get<ast::Struct>(&block)){
            for(auto& function : ptr->Content->functions){
               function.Content->marked = true; 
            }
        }
    }
}

bool still_unmarked_functions(ast::SourceFile& program){
    for(auto& block : program.Content->blocks){
        if(auto* ptr = boost::get<ast::FunctionDeclaration>(&block)){
            if(!ptr->Content->marked){
                return true;
            }
        } else if(auto* ptr = boost::get<ast::Struct>(&block)){
            for(auto& function : ptr->Content->functions){
                if(!function.Content->marked){
                    return true;
                }
            }
        }
    }

    return false;
}
