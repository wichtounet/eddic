//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "EDDIFrontEnd.hpp"
#include "SemanticalException.hpp"
#include "Options.hpp"
#include "StringPool.hpp"
#include "Type.hpp"
#include "GlobalContext.hpp"
#include "PerfsTimer.hpp"

#include "parser/SpiritParser.hpp"

#include "ast/SourceFile.hpp"

//Pass manager
#include "ast/PassManager.hpp"

//Checkers
#include "ast/StringChecker.hpp"
#include "ast/TypeChecker.hpp"

//Visitors
#include "ast/DependenciesResolver.hpp"
#include "ast/TransformerEngine.hpp"
#include "ast/WarningsEngine.hpp"
#include "ast/Printer.hpp"

#include "mtac/Compiler.hpp"
#include "mtac/RegisterAllocation.hpp"

using namespace eddic;

void check_for_main(std::shared_ptr<GlobalContext> context);
void generate_program(ast::SourceFile& program, std::shared_ptr<Configuration> configuration, Platform platform, std::shared_ptr<StringPool> pool);

std::shared_ptr<mtac::Program> EDDIFrontEnd::compile(const std::string& file, Platform platform){
    //The program to build
    ast::SourceFile program;

    //Parse the file into the program
    parser::SpiritParser parser;
    bool parsing = parser.parse(file, program); 

    //If the parsing was successfully
    if(parsing){
        set_string_pool(std::make_shared<StringPool>());

        //Read dependencies
        resolveDependencies(program, parser);
        
        //AST Passes
        generate_program(program, configuration, platform, pool);

        //If the user asked for it, print the Abstract Syntax Tree
        if(configuration->option_defined("ast") || configuration->option_defined("ast-only")){
            ast::Printer printer;
            printer.print(program);
        }
        
        //If the user wants only the AST prints, it is not necessary to compile the AST
        if(configuration->option_defined("ast-only")){
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

void generate_program(ast::SourceFile& program, std::shared_ptr<Configuration> configuration, Platform platform, std::shared_ptr<StringPool> pool){
    PerfsTimer timer("AST Passes");

    //Initialize the passes
    ast::PassManager pass_manager(platform, configuration, program, pool);
    pass_manager.init_passes();

    //Run all the passes on the program
    pass_manager.run_passes();

    //TODO The following passes can be rewritten to simple passes

    //Static analysis
    ast::checkTypes(program);

    //Transform the AST
    ast::transformAST(program);

    //Check for warnings
    ast::checkForWarnings(program, configuration);

    //Check that there is a main in the program
    check_for_main(program.Content->context);

    program.Content->context->release_references();
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
