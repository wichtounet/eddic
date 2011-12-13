//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_INTEL_X86_CODE_GENERATOR_H
#define TAC_INTEL_X86_CODE_GENERATOR_H

namespace eddic {

class AssemblyFileWriter;

namespace tac {

class Program;

struct IntelX86CodeGenerator {
    void generate(tac::Program& program, AssemblyFileWriter& writer) const ;
};

} //end of ast

} //end of eddic

#endif
