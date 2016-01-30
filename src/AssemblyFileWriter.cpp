//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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

