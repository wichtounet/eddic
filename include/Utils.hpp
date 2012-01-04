//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef UTILS_H
#define UTILS_H

#include <boost/lexical_cast.hpp>

#include <cstdlib>
#include <sstream>

namespace eddic {

/*!
 * \brief Convert a string to a number of an arbitrary type. 
 * \param text The string to convert.   
 * \return The converted text in the good type.
 */
template <typename T>
inline T toNumber (const std::string& text) {
    std::stringstream ss(text);
    T result;
    ss >> result;
    return result;
}

/*!
 * \brief Convert a string to a number of an int. This function is optimized for speed when converting int.  
 * \param text The string to convert. 
 * \return The text converted to an int. 
 */
template<>
inline int toNumber (const std::string& text) {
    //PERF: If necessary, use hand written loop
    return strtol(text.c_str(), 0, 10); 
}

/*!
 * \brief Convert a number into its string representation.  
 * \param number The number to convert. 
 * \return The string representation of the number. 
 */
template <typename T>
inline std::string toString(T number) {
    //PERF: If necessary, use karma
    return boost::lexical_cast<std::string>(number); 
}

} //end of eddic

#endif
