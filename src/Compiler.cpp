//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <cstdio>

#include "Compiler.hpp"

#include "Timer.hpp"
#include "DebugTimer.hpp"
#include "Options.hpp"

#include "StringPool.hpp"
#include "FunctionTable.hpp"
#include "SemanticalException.hpp"
#include "AssemblyFileWriter.hpp"

#include "parser/SpiritParser.hpp"
#include "ast/Program.hpp"

//Annotators
#include "DefaultValues.hpp"
#include "ContextAnnotator.hpp"
#include "FunctionsAnnotator.hpp"
#include "VariablesAnnotator.hpp"

//Checkers
#include "StringChecker.hpp"
#include "TypeChecker.hpp"

//Visitors
#include "OptimizationEngine.hpp"
#include "TransformerEngine.hpp"
#include "WarningsEngine.hpp"

//Three Address Code
#include "tac/TacCompiler.hpp"
#include "tac/Printer.hpp"
#include "tac/Program.hpp"
#include "tac/BasicBlockExtractor.hpp"
#include "tac/LivenessAnalyzer.hpp"

//Code generation
#include "asm/IntelX86CodeGenerator.hpp"

#ifdef DEBUG
static const bool debug = true;
#else
static const bool debug = false;
#endif

#define TIMER_START(name) Timer name_timer; 
#define TIMER_END(name) if(debug){std::cout << #name << " took " << name_timer.elapsed() << "s" << std::endl;}

using namespace eddic;

int Compiler::compile(const std::string& file) {
    std::cout << "Compile " << file << std::endl;

    Timer timer;

    std::string output = options["output"].as<std::string>();

    int code = 0;
    try {
        TIMER_START(parsing)

        SpiritParser parser;

        //The program to build
        ast::Program program;

        //Parse the file into the program
        bool parsing = parser.parse(file, program); 

        TIMER_END(parsing)

        if(parsing){
            //Symbol tables
            FunctionTable functionTable;
            StringPool pool;
            
            //Annotate the AST with more informations
            defineDefaultValues(program);
            defineContexts(program);
            defineVariables(program);
            defineFunctions(program, functionTable);

            //Transform the AST
            transform(program);

            //Static analysis
            checkStrings(program, pool);
            checkTypes(program);

            //Check for warnings
            checkForWarnings(program, functionTable);
            
            //Optimize the AST
            optimize(program, functionTable, pool);

            tac::Program tacProgram;

            //Generate Three-Address-Code language
            tac::TacCompiler compiler;
            compiler.compile(program, pool, tacProgram);

            //Separate into basic blocks
            tac::BasicBlockExtractor extractor;
            extractor.extract(tacProgram);

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
                execCommand("as --32 -o output.o output.asm");
                execCommand("ld -m elf_i386 output.o -o " + output);

                //Remove temporary files
                remove("output.asm");
                remove("output.o");
            }
        }
    } catch (const SemanticalException& e) {
        std::cout << e.what() << std::endl;
        code = 1;
    }

    std::cout << "Compilation took " << timer.elapsed() << "s" << std::endl;

    return code;
}

void eddic::defineDefaultValues(ast::Program& program){
    DebugTimer<debug> timer("Annotate with default values");
    DefaultValues values;
    values.fill(program);
}

void eddic::defineContexts(ast::Program& program){
    DebugTimer<debug> timer("Annotate contexts");
    ContextAnnotator annotator;
    annotator.annotate(program);
}

void eddic::defineVariables(ast::Program& program){
    DebugTimer<debug> timer("Annotate variables");
    VariablesAnnotator annotator;
    annotator.annotate(program);
}

void eddic::defineFunctions(ast::Program& program, FunctionTable& functionTable){
    DebugTimer<debug> timer("Annotate functions");
    FunctionsAnnotator annotator;
    annotator.annotate(program, functionTable);
}

void eddic::checkStrings(ast::Program& program, StringPool& pool){
    DebugTimer<debug> timer("Strings checking");
    StringChecker checker;
    checker.check(program, pool);
}

void eddic::checkTypes(ast::Program& program){
    DebugTimer<debug> timer("Types checking");
    TypeChecker checker;
    checker.check(program); 
}

void eddic::checkForWarnings(ast::Program& program, FunctionTable& table){
    DebugTimer<debug> timer("Check for warnings");
    WarningsEngine engine;
    engine.check(program, table);
}

void eddic::transform(ast::Program& program){
    DebugTimer<debug> timer("Transformation");
    TransformerEngine engine;
    engine.transform(program);
}

void eddic::optimize(ast::Program& program, FunctionTable& functionTable, StringPool& pool){
    DebugTimer<debug> timer("Optimization");
    OptimizationEngine engine;
    engine.optimize(program, functionTable, pool);
}

void eddic::execCommand(const std::string& command) {
    DebugTimer<debug> timer("Exec " + command);
    
    if(debug){
        std::cout << "eddic : exec command : " << command << std::endl;
    }

    char buffer[1024];

    FILE* stream = popen(command.c_str(), "r");

    while (fgets(buffer, 1024, stream) != NULL) {
        std::cout << buffer;
    }

    pclose(stream);
}

void eddic::warn(const std::string& warning){
    std::cout << "warning: " << warning << std::endl;
}
