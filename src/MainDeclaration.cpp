//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "MainDeclaration.hpp"

#include "il/IntermediateProgram.hpp"
#include "il/Main.hpp"

using namespace eddic;

using std::endl;
        
MainDeclaration::MainDeclaration(std::shared_ptr<Context> context, const Tok& token) : ParseNode(context, token) {}

void MainDeclaration::writeIL(IntermediateProgram& program){
    program.addInstruction(program.factory().createMainDeclaration());
}
