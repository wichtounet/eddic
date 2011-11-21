//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Methods.hpp"

#include "il/IntermediateProgram.hpp"
#include "il/PrintStringFunction.hpp"
#include "il/PrintLineFunction.hpp"
#include "il/PrintIntegerFunction.hpp"
#include "il/ConcatFunction.hpp"

using namespace eddic;

void Methods::writeIL(IntermediateProgram& program){
    program.addInstruction(program.factory().createPrintStringFunction());
    program.addInstruction(program.factory().createPrintIntegerFunction());
    program.addInstruction(program.factory().createPrintLineFunction());
    program.addInstruction(program.factory().createConcatFunction());
    program.addInstruction(program.factory().createAllocFunction());
}
