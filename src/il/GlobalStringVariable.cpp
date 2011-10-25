//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/GlobalStringVariable.hpp"

#include "AssemblyFileWriter.hpp"

using namespace eddic;

GlobalStringVariable::GlobalStringVariable(std::string name, std::string label, int size) : m_name(name), m_label(label), m_size(size) {}

void GlobalStringVariable::write(AssemblyFileWriter& writer){
    writer.stream() << ".size VS" << m_name << ", 8" << std::endl;
    writer.stream() << "VS" << m_name << ":" << std::endl;
    writer.stream() << ".long " << m_label << std::endl;
    writer.stream() << ".long " << m_size << std::endl;
}
