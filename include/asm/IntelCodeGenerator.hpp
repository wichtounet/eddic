//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef INTEL_CODE_GENERATOR_H
#define INTEL_CODE_GENERATOR_H

#include <memory>
#include <string>
#include "asm/CodeGenerator.hpp"

#include "mtac/forward.hpp"

namespace eddic {

class AssemblyFileWriter;
struct GlobalContext;
class Function;

namespace as {

/*!
 * \class IntelCodeGenerator
 * \brief Base class for code generator on Intel platform. 
 */
class IntelCodeGenerator : public CodeGenerator {
    public:
        IntelCodeGenerator(AssemblyFileWriter& writer, std::shared_ptr<GlobalContext> context);
        
        void generate(mtac::Program& program, std::shared_ptr<StringPool> pool, std::shared_ptr<FloatPool> float_pool);

    protected:
        std::shared_ptr<GlobalContext> context;

        void addGlobalVariables(std::shared_ptr<StringPool> pool, std::shared_ptr<FloatPool> float_pool);
        
        virtual void writeRuntimeSupport() = 0;
        virtual void addStandardFunctions() = 0;
        virtual void compile(mtac::Function& function) = 0;

        virtual void defineDataSection() = 0;

        virtual void declareIntArray(const std::string& name, unsigned int size) = 0;
        virtual void declareStringArray(const std::string& name, unsigned int size) = 0;
        virtual void declareFloatArray(const std::string& name, unsigned int size) = 0;

        virtual void declareIntVariable(const std::string& name, int value) = 0;
        virtual void declareStringVariable(const std::string& name, const std::string& label, int size) = 0;
        virtual void declareString(const std::string& label, const std::string& value) = 0;
        virtual void declareFloat(const std::string& label, double value) = 0;

        void output_function(const std::string& function);

        bool is_enabled_printI();
        bool is_enabled_println();
};

} //end of as

} //end of eddic

#endif
