//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ASSEMBLY_FILE_READER_H
#define ASSEMBLY_FILE_READER_H

#include <fstream>

#include "CompilerException.hpp"

namespace eddic {

class AssemblyFileWriter {
    private:
        std::ofstream m_stream;
    public:
        AssemblyFileWriter() {};
        void open(const std::string& path) ;
        void close();

        std::ofstream& stream() {
            return m_stream;
        }
};

} //end of eddic

#endif
