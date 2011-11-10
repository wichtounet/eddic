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

template <typename T>
inline T toNumber (const std::string& text) {
    std::stringstream ss(text);
    T result;
    ss >> result;
    return result;
}

template<>
inline int toNumber (const std::string& text) {
    return strtol(text.c_str(), 0, 10); //TODO Use hand-written loop (faster) if necesary
}

template <typename T>
inline std::string toString(T number) {
    return boost::lexical_cast<std::string>(number); //TODO Use karma for higher effiency
}

} //end of eddic

#endif
