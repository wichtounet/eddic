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
 * This class give access to its internal buffer that is only outputted to the file 
 * when the write() function is called. 
 */
class AssemblyFileWriter {
    private:
        std::ofstream m_stream;
        std::stringstream buffer;
	
    public:
    	/*!
    	 * Construct an AssemblyFileWriter that will output on the given path. 
    	 * \param path The path to the file. 
    	 */
        AssemblyFileWriter(const std::string& path);
        
        /*!
         * Close the file stream. 
         */
        ~AssemblyFileWriter();
        
        /*!
         * \brief Return the enclosing stream. 
         * This method should be used to output assembly instruction to the file. 
         * \return A reference to the enclosing stream. 
         */
        std::stringstream& stream();
        
        /*!
         * Output the buffer to the file. 
         */
        void write();
};

} //end of eddic

#endif
