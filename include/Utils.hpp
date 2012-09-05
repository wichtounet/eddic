//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef UTILS_H
#define UTILS_H

#include "boost_cfg.hpp"
#include <boost/lexical_cast.hpp>

#include <vector>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <algorithm>

namespace eddic {

template<typename T, typename Function>
Function for_each(std::vector<T>& values, Function f){
    return std::for_each(values.begin(), values.end(), f);
}

template<typename T>
bool are_equals(const std::vector<T>& a, const std::vector<T>& b){
    if(a.size() != b.size()){
        return false;
    }

    for(std::size_t i = 0; i < a.size(); ++i){
        if(a[i] != b[i]){
            return false;
        }
    }

    return true;
}

/*!
 * \brief Find the index of an element inside a vector. 
 * \param vector The vector to search into.   
 * \param search The element to search into the vector.   
 * \return The index of the element in the vector..
 * \throw runtime_error If the element is not contained in the vector. 
 */
template<typename T>
unsigned int index(const std::vector<T>& vector, T& search){
    for(unsigned int i = 0; i < vector.size(); ++i){
        if(vector[i] == search){
            return i;
        }
    }

    throw std::runtime_error("The element is not contained in the vector");
}

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

bool has_extension(const std::string& file, const std::string& extension);

bool file_exists(const std::string& file);

std::string execCommand(const std::string& command);

bool isPowerOfTwo (int x);

int powerOfTwo(int x);

} //end of eddic

#endif
