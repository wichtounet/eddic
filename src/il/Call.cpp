//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/Call.hpp"
#include "AssemblyFileWriter.hpp"

using namespace eddic;

Call::Call(const std::string& function) : m_function(function) {}

void Call::write(AssemblyFileWriter& writer) const {
    writer.stream() << "call " << m_function << std::endl;
}
