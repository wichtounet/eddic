//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include <memory>

#include "mtac/forward.hpp"

namespace eddic {

class FloatPool;
struct StringPool;
class AssemblyFileWriter;

namespace as {

/*!
 * \class CodeGenerator
 * Generates code for the backend from the LTAC Program. 
 */
class CodeGenerator {
    public:
        /*!
         * Constructs a CodeGenerator. 
         * \param The AssemblyFileWriter to use to output code to. 
         */
        CodeGenerator(AssemblyFileWriter& writer, mtac::Program& program);
        
        /*!
         * Generates codes from the LTAC Program. 
         * \param program The source LTAC Program. 
         * \param pool The string pool to use. 
         */
        virtual void generate(std::shared_ptr<StringPool> pool, std::shared_ptr<FloatPool> float_pool) = 0;

    protected:
        AssemblyFileWriter& writer;
        mtac::Program& program; 
};

} //end of as

} //end of eddic

#endif
