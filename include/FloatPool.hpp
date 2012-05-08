//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef FLOAT_POOL_H
#define FLOAT_POOL_H

#include <string>
#include <unordered_map>

namespace eddic {

/*!
 * \class FloatPool
 * \brief The float pool of the program. 
 * All the floats are stored and referred only by an index.  
 */
class FloatPool {
    private:
        std::unordered_map<double, std::string> pool;
        unsigned int currentString;

    public:
        FloatPool();

        FloatPool(const FloatPool& rhs) = delete;
        FloatPool& operator=(const FloatPool& rhs) = delete;

        std::string label(double value);
        double value(const std::string& label) const ;

        std::unordered_map<double, std::string> get_pool() const;
};

} //end of eddic

#endif
