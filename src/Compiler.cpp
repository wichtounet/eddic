//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#define TIMER_START(name) Timer name_timer; 
#define TIMER_END(name) std::cout << #name << " took " << name_timer.elapsed() << "s" << std::endl;

#include <iostream>
#include <cstdio>

#include "Compiler.hpp"

#include "Timer.hpp"
#include "Options.hpp"

#include "StringPool.hpp"
/*
#include "MainDeclaration.hpp"
#include "Methods.hpp"
#include "il/IntermediateProgram.hpp"
*/

#include "ast/Program.hpp"

#include "ContextAnnotator.hpp"
#include "VariableChecker.hpp"
#include "StringChecker.hpp"

#include "parser/SpiritParser.hpp"

using std::string;
using std::cout;
using std::endl;

using namespace eddic;

void defineContexts(ASTProgram& program);

void checkVariables(ASTProgram& program);
void checkStrings(ASTProgram& program, std::shared_ptr<StringPool> pool);

void execCommand(const string& command);

int Compiler::compile(const string& file) {
    cout << "Compile " << file << endl;

    Timer timer;

    string output = Options::get(ValueOption::OUTPUT);

    int code = 0;
    try {
        TIMER_START(parsing)

        SpiritParser parser;

        //The program to build
        ASTProgram program;

        //Parse the file into the program
        bool parsing = parser.parse(file, program); 

        TIMER_END(parsing)

        if(parsing){
            defineContexts(program);
              
            auto pool = std::make_shared<StringPool>();

            //Semantical analysis
            checkVariables(program);
            checkStrings(program, pool);

            //TODO program->checkFunctions(*program);

            //TODO Add things to the program (pool, main, methods)

            /*        

              program->addFirst(std::shared_ptr<ParseNode>(new MainDeclaration(program->context(), parser.getLexer().getDefaultToken())));
              program->addLast(std::shared_ptr<ParseNode>(new Methods(program->context(), parser.getLexer().getDefaultToken())));
              program->addLast(pool);

            //Optimize the parse tree
            program->optimize();

            //Write Intermediate representation of the parse tree
            IntermediateProgram il;
            program->writeIL(il);

            //Write assembly code
            writer.open("output.asm");
            il.writeAsm(writer);

            if(!Options::isSet(BooleanOption::ASSEMBLY_ONLY)){
            execCommand("as --32 -o output.o output.asm");

            string ldCommand = "gcc -m32 -static -o ";
            ldCommand += output;
            ldCommand += " output.o -lc";

            execCommand(ldCommand);

            //Remove temporary files
            remove("output.asm");
            remove("output.o");
            }*/

        }
    } catch (const CompilerException& e) {
        cout << e.what() << endl;
        code = 1;
    }

    //Close input and output
    //writer.close();

    cout << "Compilation took " << timer.elapsed() << "s" << endl;

    return code;
}

void defineContexts(ASTProgram& program){
    TIMER_START(contexts)
    ContextAnnotator annotator;
    annotator.annotate(program);
    TIMER_END(contexts);
}

void checkVariables(ASTProgram& program){
    TIMER_START(check_variables)
    VariableChecker checker;
    checker.check(program);
    TIMER_END(check_variables)
}

void checkStrings(ASTProgram& program, std::shared_ptr<StringPool> pool){
    TIMER_START(check_strings)
    StringChecker checker;
    checker.check(program, pool);
    TIMER_END(check_strings)
}

void execCommand(const string& command) {
    cout << "eddic : exec command : " << command << endl;

    char buffer[1024];

    FILE* stream = popen(command.c_str(), "r");

    while (fgets(buffer, 1024, stream) != NULL) {
        cout << buffer;
    }

    pclose(stream);
}
