//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_INTEL_X86_CODE_GENERATOR_H
#define TAC_INTEL_X86_CODE_GENERATOR_H

#include <string>

#include "tac/Program.hpp"

namespace eddic {

class AssemblyFileWriter;
class Context;

namespace tac {

struct StatementCompiler;

class IntelX86CodeGenerator {
    public:
        IntelX86CodeGenerator(AssemblyFileWriter& writer);

        void addStandardFunctions();
        void writeRuntimeSupport();
        void addGlobalVariables(std::shared_ptr<GlobalContext> context);
        void computeBlockUsage(std::shared_ptr<tac::Function> function, StatementCompiler& compiler);
        void computeLiveness(std::shared_ptr<tac::Function> function);
        void generate(tac::Program& program);
        void compile(std::shared_ptr<tac::Function> function);
        void compile(std::shared_ptr<tac::BasicBlock> block, StatementCompiler& compiler);

    private:
        AssemblyFileWriter& writer;
};

} //end of ast

} //end of eddic

#endif
