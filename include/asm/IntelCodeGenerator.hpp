//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef INTEL_CODE_GENERATOR_H
#define INTEL_CODE_GENERATOR_H

#include <memory>
#include <string>

#include <boost/utility/enable_if.hpp>

#include "tac/Program.hpp"

#include "asm/CodeGenerator.hpp"

namespace eddic {

class AssemblyFileWriter;
class GlobalContext;

namespace as {

/*!
 * \class IntelCodeGenerator
 * \brief Base class for code generator on Intel platform. 
 */
class IntelCodeGenerator : public CodeGenerator {
    public:
        IntelCodeGenerator(AssemblyFileWriter& writer);
        
        void generate(tac::Program& program, StringPool& pool, SymbolTable& table);

    protected:
        void addGlobalVariables(std::shared_ptr<GlobalContext> context, StringPool& pool);
        
        virtual void writeRuntimeSupport(SymbolTable& table) = 0;
        virtual void addStandardFunctions() = 0;
        virtual void compile(std::shared_ptr<tac::Function> function) = 0;

        virtual void defineDataSection() = 0;

        virtual void declareIntArray(const std::string& name, unsigned int size) = 0;
        virtual void declareStringArray(const std::string& name, unsigned int size) = 0;
        virtual void declareFloatArray(const std::string& name, unsigned int size) = 0;

        virtual void declareIntVariable(const std::string& name, int value) = 0;
        virtual void declareStringVariable(const std::string& name, const std::string& label, int size) = 0;
        virtual void declareString(const std::string& label, const std::string& value) = 0;
};

//Provide utility for registers

template<typename Reg>
inline typename boost::enable_if<boost::is_enum<Reg>, std::ostream>::type&
operator<<(std::ostream& os, Reg reg){
    return os << regToString(reg);
}

template<typename Reg>
inline typename boost::enable_if<boost::is_enum<Reg>, std::string>::type
operator+(const char* left, Reg right) {
    return left + regToString(right);
}

template<typename Reg>
inline typename boost::enable_if<boost::is_enum<Reg>, std::string>::type
operator+(std::string left, Reg right) {
    return left + regToString(right);
}

} //end of as

} //end of eddic

#endif
