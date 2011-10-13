//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "AssemblyFileWriter.hpp"

using std::string;
using std::ios;
using std::endl;

using namespace eddic;

void AssemblyFileWriter::open(const std::string& path) {
    m_stream.open(path.c_str());

    if (!m_stream) {
        //TODO throw CompilerException("Unable to open the output file");
    }
}

void AssemblyFileWriter::close() {
    m_stream.close();
}

std::ofstream& AssemblyFileWriter::stream() {
    return m_stream;
}
