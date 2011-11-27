//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/GlobalIntVariable.hpp"
#include "AssemblyFileWriter.hpp"

using namespace eddic;

GlobalIntVariable::GlobalIntVariable(const std::string& name, int value) : m_name(name), m_value(value) {}

void GlobalIntVariable::write(AssemblyFileWriter& writer){
    writer.stream() << ".size VI" << m_name << ", 4" << std::endl;
    writer.stream() << "VI" << m_name << ":" << std::endl;
    writer.stream() << ".long " << m_value << std::endl;
}
