//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef STRING_POOL_H
#define STRING_POOL_H

#include <string>
#include <unordered_map>

namespace eddic {

/*!
 * \class StringPool
 * \brief The string pool of the program. 
 * All the strings are stored and referred only by an index.  
 */
class StringPool {
    private:
        std::unordered_map<std::string, std::string> pool;
        unsigned int currentString;

    public:
        StringPool();
        StringPool(const StringPool& rhs) = delete;

        std::string label(const std::string& value);

        std::unordered_map<std::string, std::string> getPool() const;
};

} //end of eddic

#endif
