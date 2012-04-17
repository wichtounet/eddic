//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <cstdio>

#include "Compiler.hpp"
#include "Target.hpp"
#include "Utils.hpp"
#include "DebugStopWatch.hpp"
#include "Options.hpp"
#include "StringPool.hpp"
#include "SymbolTable.hpp"
#include "SemanticalException.hpp"
#include "AssemblyFileWriter.hpp"
#include "Assembler.hpp"
#include "RegisterAllocation.hpp"

#include "FrontEnds.hpp"

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

//Three Address Code
#include "tac/Program.hpp"
#include "tac/Compiler.hpp"
#include "tac/BasicBlockExtractor.hpp"
#include "tac/TemporaryAllocator.hpp"
#include "tac/LivenessAnalyzer.hpp"
#include "tac/Optimizer.hpp"
#include "tac/Printer.hpp"

//Code generation
#include "asm/CodeGeneratorFactory.hpp"

//32 bits by default
eddic::Platform eddic::platform = Platform::INTEL_X86;

#ifdef DEBUG
static const bool debug = true;
#else
static const bool debug = false;
#endif

#define TIMER_START(name) StopWatch name_timer; 
#define TIMER_END(name) if(debug){std::cout << #name << " took " << name_timer.elapsed() << "s" << std::endl;}

using namespace eddic;

int Compiler::compile(const std::string& file) {
    if(!option_defined("quiet")){
        std::cout << "Compile " << file << std::endl;
    }

    if(TargetDetermined && Target64){
        platform = Platform::INTEL_X86_64;
    }

    if(option_defined("32")){
        platform = Platform::INTEL_X86;
    }
    
    if(option_defined("64")){
        platform = Platform::INTEL_X86_64;
    }

    StopWatch timer;
    
    int code = compileOnly(file, platform);

    if(!option_defined("quiet")){
        std::cout << "Compilation took " << timer.elapsed() << "ms" << std::endl;
    }

    return code;
}

int Compiler::compileOnly(const std::string& file, Platform platform) {
    //Reset the symbol table
    symbols.reset();

    //Make sure that the file exists 
    if(!file_exists(file)){
        std::cout << "The file \"" + file + "\" does not exists" << std::endl;

        return false;
    }

    auto front_end = get_front_end(file);

    if(!front_end){
        std::cout << "The file \"" + file + "\" cannot be compiled using eddic" << std::endl;

        return false;
    }

    std::string output = option_value("output");

    int code = 0;
    try {
        TIMER_START(parsing)

        parser::SpiritParser parser;

        //The program to build
        ast::SourceFile program;

        //Parse the file into the program
        bool parsing = parser.parse(file, program); 

        TIMER_END(parsing)

        //If the parsing was sucessfully
        if(parsing){
            StringPool pool;

            //Read dependencies
            resolveDependencies(program, parser);

            //Apply some cleaning transformations
            ast::cleanAST(program);

            //Annotate the AST with more informations
            ast::defineDefaultValues(program);

            //Fill the string pool
            ast::checkStrings(program, pool);

            //Add some more informations to the AST
            ast::defineStructures(program);
            ast::defineContexts(program);
            ast::defineVariables(program);
            ast::defineFunctions(program);
            
            //Allocate registers to params
            allocateParams();

            //Transform the AST
            ast::transformAST(program);

            //Static analysis
            ast::checkTypes(program);

            //Check for warnings
            ast::checkForWarnings(program);

            //Check that there is a main in the program
            checkForMain();

            //Optimize the AST
            ast::optimizeAST(program, pool);

            //If the user asked for it, print the Abstract Syntax Tree
            if(option_defined("ast") || option_defined("ast-only")){
                ast::DebugVisitor()(program);
            }

            //If necessary, continue the compilation process
            if(!option_defined("ast-only")){
                tac::Program tacProgram;

                //Generate Three-Address-Code language
                tac::Compiler compiler;
                compiler.compile(program, pool, tacProgram);

                //Separate into basic blocks
                tac::BasicBlockExtractor extractor;
                extractor.extract(tacProgram);

                //Allocate storage for the temporaries that need to be stored
                tac::TemporaryAllocator allocator;
                allocator.allocate(tacProgram);

                tac::Optimizer optimizer;
                optimizer.optimize(tacProgram, pool);

                //If asked by the user, print the Three Address code representation
                if(option_defined("tac") || option_defined("tac-only")){
                    tac::Printer printer;
                    printer.print(tacProgram);
                }

                //If necessary, continue the compilation process
                if(!option_defined("tac-only")){
                    //Compute liveness of variables
                    tac::LivenessAnalyzer liveness;
                    liveness.compute(tacProgram);

                    //Generate assembly from TAC
                    AssemblyFileWriter writer("output.asm");

                    as::CodeGeneratorFactory factory;
                    auto generator = factory.get(platform, writer);
                    generator->generate(tacProgram, pool); 
                    writer.write(); 

                    //If it's necessary, assemble and link the assembly
                    if(!option_defined("assembly")){
                        assemble(platform, output, option_defined("debug"), option_defined("verbose"));

                        //Remove temporary files
                        if(!option_defined("keep")){
                            remove("output.asm");
                        }

                        remove("output.o");
                    }
                }
            }
        }
    } catch (const SemanticalException& e) {
        if(!option_defined("quiet")){
            if(e.position()){
                auto& position = *e.position();

                std::cout << position.file << ":" << position.line << ":" << " error: " << e.what() << std::endl;
            } else {
                std::cout << e.what() << std::endl;
            }
        }

        code = 1;
    }

    return code;
}

void eddic::checkForMain(){
    if(!symbols.exists("main")){
        throw SemanticalException("Your program must contain a main function"); 
    }

    auto function = symbols.getFunction("main");

    if(function->parameters.size() > 1){
        throw SemanticalException("The signature of your main function is not valid");
    }

    if(function->parameters.size() == 1){
        auto type = function->parameters[0].paramType;
       
        if(type.base() != BaseType::STRING || !type.isArray()){
            throw SemanticalException("The signature of your main function is not valid");
        }
    }
}
