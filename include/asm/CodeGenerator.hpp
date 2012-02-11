//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "tac/Program.hpp"

#include <memory>

namespace eddic {

class AssemblyFileWriter;
class FunctionTable;
class GlobalContext;
class StringPool;

namespace as {

class StatementCompiler;

class CodeGenerator {
    public:
        CodeGenerator(AssemblyFileWriter& writer);
        
        //Entry point
        void generate(tac::Program& program, StringPool& pool, FunctionTable& table);

    protected:
        virtual void writeRuntimeSupport(FunctionTable& table) = 0;
        virtual void addStandardFunctions() = 0;
        
        void addGlobalVariables(std::shared_ptr<GlobalContext> context, StringPool& pool);
        
        void compile(std::shared_ptr<tac::Function> function);
        void computeBlockUsage(std::shared_ptr<tac::Function> function, StatementCompiler& compiler);
        void computeLiveness(std::shared_ptr<tac::Function> function);
        void compile(std::shared_ptr<tac::BasicBlock> block, StatementCompiler& compiler);

    protected:
        AssemblyFileWriter& writer;
};

} //end of as

} //end of eddic

#endif
