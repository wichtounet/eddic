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
#include "Variables.hpp"
#include "Compiler.hpp"
#include "Nodes.hpp"
#include "Parser.hpp"

using std::string;
using std::cout;
using std::endl;

using namespace eddic;

void execCommand(string command);

int Compiler::compile(string file) {
    cout << "Compile " << file << endl;

    Timer timer;

    string output = Options::get(OUTPUT);

    int code = 0;
    try {
        lexer.lex(file);

        Parser parser(lexer);

        Program* program = parser.parse();

        Variables variables;
        StringPool* pool = new StringPool();

        //Semantical analysis
        check(program, variables);
        checkStrings(program, *pool);

        //Optimize the parse tree
        program->optimize();

        writer.open("output.asm");
        compile(program, *pool);

        variables.write(writer);

        delete program;
        delete pool;

        string asCommand = "as --32 -o output.o output.asm";

        execCommand(asCommand);

        string ldCommand = "ld -static -m elf_i386 -o ";
        ldCommand += output;
        ldCommand += " output.o -lc";

        execCommand(ldCommand);

        //Remove temporary files
//        remove("output.asm");
//        remove("output.o");
    } catch (CompilerException e) {
        cout << e.what() << endl;
        code = 1;
    }

    lexer.close();
    writer.close();

    if(code != 0) {
        remove(output.c_str());
    }

    cout << "Compilation took " << timer.elapsed() << "ms" << endl;

    return code;
}

void execCommand(string command) {
    cout << "Exec command : " << command << endl;

    char buffer[1024];

    FILE* stream = popen(command.c_str(), "r");

    while(fgets(buffer, 1024, stream) != NULL) {
        cout << buffer;
    }

    pclose(stream);
}

void Compiler::compile(Program* program, StringPool& pool) {
    writer.writeHeader();

    program->write(writer);

    writer.writeEnd();

    pool.write(writer);
}

void Compiler::check(Program* program, Variables& variables) {
    NodeIterator it = program->begin();
    NodeIterator end = program->end();

    for( ; it != end; ++it) {
        ParseNode* node = *it;

        node->checkVariables(variables);
    }
}

void Compiler::checkStrings(Program* program, StringPool& pool) {
    NodeIterator it = program->begin();
    NodeIterator end = program->end();

    for( ; it != end; ++it) {
        ParseNode* node = *it;

        node->checkStrings(pool);
    }
}
