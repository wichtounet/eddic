//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef INTEL_X86_CODE_GENERATOR_H
#define INTEL_X86_CODE_GENERATOR_H

#include "asm/IntelCodeGenerator.hpp"

namespace eddic {

namespace as {

class StatementCompiler;

class IntelX86CodeGenerator : public IntelCodeGenerator {
    public:
        IntelX86CodeGenerator(AssemblyFileWriter& writer);
        
        void writeRuntimeSupport(FunctionTable& table);
        void addStandardFunctions();
        void compile(std::shared_ptr<tac::Function> function);

        void defineDataSection();
        void declareIntArray(const std::string& name, unsigned int size);
        void declareStringArray(const std::string& name, unsigned int size);
        void declareIntVariable(const std::string& name, int value);
        void declareStringVariable(const std::string& name, const std::string& label, int size);
        void declareString(const std::string& label, const std::string& value);
        
        
        
        void computeBlockUsage(std::shared_ptr<tac::Function> function, StatementCompiler& compiler);
        void computeLiveness(std::shared_ptr<tac::Function> function);
        void compile(std::shared_ptr<tac::BasicBlock> block, StatementCompiler& compiler);
};

} //end of as

} //end of eddic

#endif
