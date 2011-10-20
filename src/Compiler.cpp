//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <cstdio>

#include "Timer.hpp"
#include "Options.hpp"
#include "StringPool.hpp"
#include "Compiler.hpp"
#include "Program.hpp"
#include "Parser.hpp"
#include "MainDeclaration.hpp"
#include "Methods.hpp"
#include "il/IntermediateProgram.hpp"

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
        lexer.lex(file);

        Parser parser(lexer);

        std::shared_ptr<Program> program = parser.parse();

        std::shared_ptr<StringPool> pool(new StringPool(program->context()));

        program->addFirst(std::shared_ptr<ParseNode>(new MainDeclaration(program->context())));
        program->addLast(std::shared_ptr<ParseNode>(new Methods(program->context())));
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
        }
    } catch (const CompilerException& e) {
        cout << e.what() << endl;
        code = 1;
    }

    //Close input and output
    lexer.close();
    writer.close();

    cout << "Compilation took " << timer.elapsed() << "ms" << endl;

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
