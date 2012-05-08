//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "ltac/Program.hpp"
#include "FloatPool.hpp"

#include <memory>

namespace eddic {

class StringPool;
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
        CodeGenerator(AssemblyFileWriter& writer);
        
        /*!
         * Generates codes from the LTAC Program. 
         * \param program The source LTAC Program. 
         * \param pool The string pool to use. 
         */
        virtual void generate(ltac::Program& program, StringPool& pool, std::shared_ptr<FloatPool> float_pool) = 0;

    protected:
        AssemblyFileWriter& writer;
};

} //end of as

} //end of eddic

#endif
