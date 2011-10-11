//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/Jump.hpp"

#include "AssemblyFileWriter.hpp"

using namespace eddic;

Jump::Jump(std::string label) : m_label(label) {}

void Jump::write(AssemblyFileWriter& writer){
    writer.stream() << "jmp " << m_label << std::endl;
}
