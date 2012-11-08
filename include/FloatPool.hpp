//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
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

        /*!
         * \brief Return the label for the given value. 
         * If the given value is not in the pool, it will be inserted with a new label. 
         * \n\n \b Complexity : O(1)
         * \param value The string we want to search in the pool. 
         * \return The label associated with the given value. 
         */
        std::string label(double value);
        
        /*!
         * \brief Return the value for the given label. 
         * This function should only be used for existing labels.
         * \n\n \b Complexity : O(n)
         * \param label The label we want to search in the pool. 
         * \return The value associated with the given label. 
         */
        double value(const std::string& label) const ;

        std::unordered_map<double, std::string> get_pool() const;
};

} //end of eddic

#endif
