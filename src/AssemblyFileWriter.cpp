//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "AssemblyFileWriter.hpp"

#include "SemanticalException.hpp"

using namespace eddic;

AssemblyFileWriter::AssemblyFileWriter(const std::string& path) {
    m_stream.open(path.c_str());

    if (!m_stream) {
        throw SemanticalException("Unable to open the output file");
    }
}

AssemblyFileWriter::~AssemblyFileWriter() {
}

std::ostream& AssemblyFileWriter::stream() {
    return m_stream;
}

