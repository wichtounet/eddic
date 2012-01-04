//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ASSEMBLY_FILE_READER_H
#define ASSEMBLY_FILE_READER_H

#include <sstream>
#include <fstream>

namespace eddic {

/*!
 * \class AssemblyFileWriter
 * \brief A simple writer to append assembly code to the assembly output file. 
 */
class AssemblyFileWriter {
    private:
        std::ofstream m_stream;
        std::stringstream buffer;
	
    public:		
        AssemblyFileWriter(const std::string& path);
        ~AssemblyFileWriter();
        
        std::stringstream& stream();
        void write();
};

} //end of eddic

#endif
