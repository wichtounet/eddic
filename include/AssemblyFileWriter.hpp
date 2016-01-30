//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef ASSEMBLY_FILE_WRITER_H
#define ASSEMBLY_FILE_WRITER_H

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
        std::ostream& stream();
};

} //end of eddic

#endif
