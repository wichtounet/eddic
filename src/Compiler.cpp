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
#include "Options.hpp"
/*#include "StringPool.hpp"
#include "Program.hpp"
#include "MainDeclaration.hpp"
#include "Methods.hpp"
#include "il/IntermediateProgram.hpp"*/

#include "ast/Program.hpp"

#include "parser/SpiritParser.hpp"

using std::string;
using std::cout;
using std::endl;

using namespace eddic;

void execCommand(const string& command);

int Compiler::compile(const string& file) {
    cout << "Compile " << file << endl;

    Timer timer;

    string output = Options::get(ValueOption::OUTPUT);

    int code = 0;
    try {
        SpiritParser parser;

        //The program to build
        ASTProgram program;

        //Parse the file into the program
        if(parser.parse(file, program)){
            std::cout << program.blocks.size() << std::endl;

            //TODO Add the contexts to the program

            //TODO Add things to the program (pool, main, methods)

            //TODO Semantical analysis

            /*        
                      std::shared_ptr<StringPool> pool(new StringPool(program->context(), parser.getLexer().getDefaultToken()));

                      program->addFirst(std::shared_ptr<ParseNode>(new MainDeclaration(program->context(), parser.getLexer().getDefaultToken())));
                      program->addLast(std::shared_ptr<ParseNode>(new Methods(program->context(), parser.getLexer().getDefaultToken())));
                      program->addLast(pool);

            //Semantical analysis
            program->checkVariables();
            program->checkStrings(*pool);
            program->checkFunctions(*program);

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

void execCommand(const string& command) {
    cout << "eddic : exec command : " << command << endl;

    char buffer[1024];

    FILE* stream = popen(command.c_str(), "r");

    while (fgets(buffer, 1024, stream) != NULL) {
        cout << buffer;
    }

    pclose(stream);
}
