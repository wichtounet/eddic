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

#include "parser/SpiritParser.hpp"
#include "ast/SourceFile.hpp"

//Annotators
#include "DefaultValues.hpp"
#include "ContextAnnotator.hpp"
#include "FunctionsAnnotator.hpp"
#include "VariablesAnnotator.hpp"

//Checkers
#include "StringChecker.hpp"
#include "TypeChecker.hpp"

//Visitors
#include "DependenciesResolver.hpp"
#include "OptimizationEngine.hpp"
#include "TransformerEngine.hpp"
#include "WarningsEngine.hpp"
#include "DebugVisitor.hpp"

//Three Address Code
#include "tac/Program.hpp"
#include "tac/Compiler.hpp"
#include "tac/BasicBlockExtractor.hpp"
#include "tac/TemporaryAllocator.hpp"
#include "tac/LivenessAnalyzer.hpp"
#include "tac/Optimizer.hpp"
#include "tac/Printer.hpp"

//Code generation
#include "asm/IntelX86CodeGenerator.hpp"

#ifdef DEBUG
static const bool debug = true;
#else
static const bool debug = false;
#endif

#define TIMER_START(name) StopWatch name_timer; 
#define TIMER_END(name) if(debug){std::cout << #name << " took " << name_timer.elapsed() << "s" << std::endl;}

using namespace eddic;

void exec(const std::string& command);

int Compiler::compile(const std::string& file) {
    std::cout << "Compile " << file << std::endl;

    if(TargetDetermined && Target64){
        std::cout << "Warning : Looks like you're running a 64 bit system. This compiler only outputs 32 bits assembly." << std::endl; 
    }

    StopWatch timer;
    
    int code = compileOnly(file);

    std::cout << "Compilation took " << timer.elapsed() << "s" << std::endl;

    return code;
}

int Compiler::compileOnly(const std::string& file) {
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
            
            //Optimize the AST
            optimize(program, functionTable, pool);

            DebugVisitor()(program);
    
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

            tac::Printer printer;
            printer.print(tacProgram);

            tac::Optimizer optimizer;
            optimizer.optimize(tacProgram);

            //Compute liveness of variables
            tac::LivenessAnalyzer liveness;
            liveness.compute(tacProgram);

            //Generate assembly from TAC
            AssemblyFileWriter writer("output.asm");
            as::IntelX86CodeGenerator generator(writer);
            generator.generate(tacProgram, pool); 
            writer.write(); 

            //If it's necessary, assemble and link the assembly
            if(!options.count("assembly")){
                exec("as --32 -o output.o output.asm");
                exec("ld -m elf_i386 output.o -o " + output);

                //Remove temporary files
                remove("output.asm");
                remove("output.o");
            }
        }
    } catch (const SemanticalException& e) {
        std::cout << e.what() << std::endl;
        code = 1;
    }

    return code;
}

void eddic::defineDefaultValues(ast::SourceFile& program){
    DebugStopWatch<debug> timer("Annotate with default values");
    DefaultValues values;
    values.fill(program);
}

void eddic::defineContexts(ast::SourceFile& program){
    DebugStopWatch<debug> timer("Annotate contexts");
    ContextAnnotator annotator;
    annotator.annotate(program);
}

void eddic::defineVariables(ast::SourceFile& program){
    DebugStopWatch<debug> timer("Annotate variables");
    VariablesAnnotator annotator;
    annotator.annotate(program);
}

void eddic::defineFunctions(ast::SourceFile& program, FunctionTable& functionTable){
    DebugStopWatch<debug> timer("Annotate functions");
    FunctionsAnnotator annotator;
    annotator.annotate(program, functionTable);
}

void eddic::checkStrings(ast::SourceFile& program, StringPool& pool){
    DebugStopWatch<debug> timer("Strings checking");
    StringChecker checker;
    checker.check(program, pool);
}

void eddic::checkTypes(ast::SourceFile& program){
    DebugStopWatch<debug> timer("Types checking");
    TypeChecker checker;
    checker.check(program); 
}

void eddic::checkForWarnings(ast::SourceFile& program, FunctionTable& table){
    DebugStopWatch<debug> timer("Check for warnings");
    WarningsEngine engine;
    engine.check(program, table);
}

void eddic::clean(ast::SourceFile& program){
    DebugStopWatch<debug> timer("Cleaning");
    TransformerEngine engine;
    engine.clean(program);
}

void eddic::transform(ast::SourceFile& program){
    DebugStopWatch<debug> timer("Transformation");
    TransformerEngine engine;
    engine.transform(program);
}

void eddic::optimize(ast::SourceFile& program, FunctionTable& functionTable, StringPool& pool){
    DebugStopWatch<debug> timer("Optimization");
    OptimizationEngine engine;
    engine.optimize(program, functionTable, pool);
}

void eddic::includeDependencies(ast::SourceFile& sourceFile, parser::SpiritParser& parser){
    DebugStopWatch<debug> timer("Resolve dependencies");
    DependenciesResolver resolver(parser);
    resolver.resolve(sourceFile);
}

void exec(const std::string& command) {
    DebugStopWatch<debug> timer("Exec " + command);
    
    if(debug){
        std::cout << "eddic : exec command : " << command << std::endl;
    }

    std::string result = execCommand(command);

    if(result.size() > 0){
        std::cout << result << std::endl;
    }
}

void eddic::warn(const std::string& warning){
    std::cout << "warning: " << warning << std::endl;
}
