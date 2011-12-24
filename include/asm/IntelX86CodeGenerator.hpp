//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ASM_INTEL_X86_CODE_GENERATOR_H
#define ASM_INTEL_X86_CODE_GENERATOR_H

#include <string>

#include "tac/Program.hpp"

namespace eddic {

class AssemblyFileWriter;
class Context;
class StringPool;

namespace as {

struct StatementCompiler;

class IntelX86CodeGenerator {
    public:
        IntelX86CodeGenerator(AssemblyFileWriter& writer);

        void addStandardFunctions();
        void writeRuntimeSupport();
        void addGlobalVariables(std::shared_ptr<GlobalContext> context, StringPool& pool);
        void computeBlockUsage(std::shared_ptr<tac::Function> function, StatementCompiler& compiler);
        void computeLiveness(std::shared_ptr<tac::Function> function);
        void compile(std::shared_ptr<tac::Function> function);
        void compile(std::shared_ptr<tac::BasicBlock> block, StatementCompiler& compiler);
        
        void generate(tac::Program& program, StringPool& pool);

    private:
        AssemblyFileWriter& writer;
};

} //end of as

} //end of eddic

#endif
