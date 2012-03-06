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
#include "FunctionTable.hpp"
#include "SemanticalException.hpp"
#include "AssemblyFileWriter.hpp"
#include "Assembler.hpp"

#include "parser/SpiritParser.hpp"

#include "ast/SourceFile.hpp"

//Annotators
#include "ast/DefaultValues.hpp"
#include "ast/ContextAnnotator.hpp"
#include "ast/FunctionsAnnotator.hpp"
#include "ast/VariablesAnnotator.hpp"

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
    std::cout << "Compile " << file << std::endl;

    if(TargetDetermined && Target64){
        platform = Platform::INTEL_X86_64;
    }

    if(options.count("32")){
        platform = Platform::INTEL_X86;
    }
    
    if(options.count("64")){
        platform = Platform::INTEL_X86_64;
    }

    StopWatch timer;
    
    int code = compileOnly(file, platform);

    std::cout << "Compilation took " << timer.elapsed() << "s" << std::endl;

    return code;
}

void assemble(Platform platform, const std::string& output);
void assembleWithDebug(Platform platform, const std::string& output);

int Compiler::compileOnly(const std::string& file, Platform platform) {
    std::string output = options["output"].as<std::string>();

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
            //Symbol tables
            FunctionTable functionTable;
            StringPool pool;

            //Read dependencies
            includeDependencies(program, parser);

            //Apply some cleaning transformations
            clean(program);

            //Annotate the AST with more informations
            defineDefaultValues(program);

            //Fill the string pool
            checkStrings(program, pool);

            //Add some more informations to the AST
            defineContexts(program);
            defineVariables(program);
            defineFunctions(program, functionTable);

            //Transform the AST
            transform(program);

            //Static analysis
            checkTypes(program);

            //Check for warnings
            checkForWarnings(program, functionTable);

            //Check that there is a main in the program
            checkForMain(functionTable);

            //Optimize the AST
            optimize(program, functionTable, pool);

            //If the user asked for it, print the Abstract Syntax Tree
            if(options.count("ast") || options.count("ast-only")){
                ast::DebugVisitor()(program);
            }

            //If necessary, continue the compilation process
            if(!options.count("ast-only")){
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
                if(options.count("tac") || options.count("tac-only")){
                    tac::Printer printer;
                    printer.print(tacProgram);
                }

                //If necessary, continue the compilation process
                if(!options.count("tac-only")){
                    //Compute liveness of variables
                    tac::LivenessAnalyzer liveness;
                    liveness.compute(tacProgram);

                    //Generate assembly from TAC
                    AssemblyFileWriter writer("output.asm");

                    as::CodeGeneratorFactory factory;
                    auto generator = factory.get(platform, writer);
                    generator->generate(tacProgram, pool, functionTable); 
                    writer.write(); 

                    //If it's necessary, assemble and link the assembly
                    if(!options.count("assembly")){
                        assemble(platform, output, options.count("assembly"));

                        //Remove temporary files
                        if(!options.count("keep")){
                            remove("output.asm");
                        }

                        remove("output.o");
                    }
                }
            }
        }
    } catch (const SemanticalException& e) {
        if(e.position()){
            auto& position = *e.position();

            std::cout << position.file << ":" << position.line << ":" << " error: " << e.what() << std::endl;
        } else {
            std::cout << e.what() << std::endl;
        }

        code = 1;
    }

    return code;
}

void eddic::defineDefaultValues(ast::SourceFile& program){
    DebugStopWatch<debug> timer("Annotate with default values");
    ast::DefaultValues values;
    values.fill(program);
}

void eddic::defineContexts(ast::SourceFile& program){
    DebugStopWatch<debug> timer("Annotate contexts");
    ast::ContextAnnotator annotator;
    annotator.annotate(program);
}

void eddic::defineVariables(ast::SourceFile& program){
    DebugStopWatch<debug> timer("Annotate variables");
    ast::VariablesAnnotator annotator;
    annotator.annotate(program);
}

void eddic::defineFunctions(ast::SourceFile& program, FunctionTable& functionTable){
    DebugStopWatch<debug> timer("Annotate functions");
    ast::FunctionsAnnotator annotator;
    annotator.annotate(program, functionTable);
}

void eddic::checkStrings(ast::SourceFile& program, StringPool& pool){
    DebugStopWatch<debug> timer("Strings checking");
    ast::StringChecker checker;
    checker.check(program, pool);
}

void eddic::checkTypes(ast::SourceFile& program){
    DebugStopWatch<debug> timer("Types checking");
    ast::TypeChecker checker;
    checker.check(program); 
}

void eddic::checkForWarnings(ast::SourceFile& program, FunctionTable& table){
    DebugStopWatch<debug> timer("Check for warnings");
    ast::WarningsEngine engine;
    engine.check(program, table);
}

void eddic::checkForMain(FunctionTable& table){
    if(!table.exists("main")){
        throw SemanticalException("Your program must contain a main function"); 
    }

    auto function = table.getFunction("main");

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

void eddic::clean(ast::SourceFile& program){
    DebugStopWatch<debug> timer("Cleaning");
    ast::TransformerEngine engine;
    engine.clean(program);
}

void eddic::transform(ast::SourceFile& program){
    DebugStopWatch<debug> timer("Transformation");
    ast::TransformerEngine engine;
    engine.transform(program);
}

void eddic::optimize(ast::SourceFile& program, FunctionTable& functionTable, StringPool& pool){
    DebugStopWatch<debug> timer("Optimization");
    ast::OptimizationEngine engine;
    engine.optimize(program, functionTable, pool);
}

void eddic::includeDependencies(ast::SourceFile& sourceFile, parser::SpiritParser& parser){
    DebugStopWatch<debug> timer("Resolve dependencies");
    ast::DependenciesResolver resolver(parser);
    resolver.resolve(sourceFile);
}
