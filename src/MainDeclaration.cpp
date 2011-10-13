//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "MainDeclaration.hpp"
#include "AssemblyFileWriter.hpp"

using namespace eddic;

using std::endl;
        
MainDeclaration::MainDeclaration(std::shared_ptr<Context> context, const Tok& token) : ParseNode(context, token) {}

void MainDeclaration::write(AssemblyFileWriter& writer){
    writer.stream() << ".text" << endl
                    << ".globl main" << endl
                    << "\t.type main, @function" << endl;
}
