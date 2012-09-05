//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef FRONT_END_H
#define FRONT_END_H

#include <memory>
#include <string>

#include "Platform.hpp"

#include "mtac/Program.hpp"

namespace eddic {

class StringPool;

/*!
 * \class FrontEnd
 * \brief Represent the front end part of the compilation process. 
 */
class FrontEnd {
    public:
        /*!
         * Compile the given file into its MTAC Representation. 
         * \param file The file that has to be compiled. 
         * \return The MTAC Program representing the source program. 
         */
        virtual std::shared_ptr<mtac::Program> compile(const std::string& file, Platform platform) = 0;   

        /*!
         * Set the string pool. 
         * \param pool The string pool. 
         */
        void set_string_pool(std::shared_ptr<StringPool> pool);
        
        /*!
         * Returns the string pool. 
         * \return The string pool. 
         */
        std::shared_ptr<StringPool> get_string_pool();

    protected:
        std::shared_ptr<StringPool> pool; /**< The string pool that is used during this compilation */
};

}

#endif
