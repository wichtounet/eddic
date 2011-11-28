//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/Label.hpp"
#include "AssemblyFileWriter.hpp"

using namespace eddic;

Label::Label(const std::string& name) : m_name(name) {}

void Label::write(AssemblyFileWriter& writer) const {
    writer.stream() << m_name << ":" << std::endl;
}
