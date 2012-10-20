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

/*!
 * \class IntelX86CodeGenerator
 * \brief Code generator for Intel X86 platform. 
 */
class IntelX86CodeGenerator : public IntelCodeGenerator {
    public:
        IntelX86CodeGenerator(AssemblyFileWriter& writer, std::shared_ptr<GlobalContext> context);

    protected:        
        void writeRuntimeSupport();
        void addStandardFunctions();
        void compile(std::shared_ptr<mtac::Function> function);

        /* Functions for global variables */
        void defineDataSection();
        void declareStringArray(const std::string& name, unsigned int size);
        void declareIntArray(const std::string& name, unsigned int size);
        void declareFloatArray(const std::string& name, unsigned int size);

        void declareIntVariable(const std::string& name, int value);
        void declareStringVariable(const std::string& name, const std::string& label, int size);
        void declareString(const std::string& label, const std::string& value);
        void declareFloat(const std::string& label, double value);
};

} //end of as

} //end of eddic

#endif
