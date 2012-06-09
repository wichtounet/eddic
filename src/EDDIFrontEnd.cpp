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
#include "RegisterAllocation.hpp"
#include "Options.hpp"
#include "StringPool.hpp"
#include "Type.hpp"
#include "Types.hpp"

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
#include "ast/DebugVisitor.hpp"

#include "mtac/Compiler.hpp"

using namespace eddic;

void checkForMain();

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

        //Annotate the AST with more informations
        ast::defineDefaultValues(program);

        //Fill the string pool
        ast::checkStrings(program, *pool);

        //Add some more informations to the AST
        ast::defineStructures(program);
        ast::defineContexts(program);
        ast::defineVariables(program);
        ast::defineFunctions(program);

        //Allocate registers to params
        allocateParams();

        //Static analysis
        ast::checkTypes(program);

        //Check for warnings
        ast::checkForWarnings(program);

        //Check that there is a main in the program
        checkForMain();

        //Transform the AST
        ast::transformAST(program);

        //Optimize the AST
        ast::optimizeAST(program, *pool);

        //If the user asked for it, print the Abstract Syntax Tree
        if(option_defined("ast") || option_defined("ast-only")){
            ast::DebugVisitor()(program);
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

void checkForMain(){
    if(!symbols.exists("main")){
        throw SemanticalException("Your program must contain a main function"); 
    }

    auto function = symbols.getFunction("main");

    if(function->parameters.size() > 1){
        throw SemanticalException("The signature of your main function is not valid");
    }

    if(function->parameters.size() == 1){
        auto type = function->parameters[0].paramType;
       
        if(type->element_type() != STRING || !type->is_array()){
            throw SemanticalException("The signature of your main function is not valid");
        }
    }
}
