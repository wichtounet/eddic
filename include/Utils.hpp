//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef UTILS_H
#define UTILS_H

#include <sstream>

namespace eddic {

template <typename T>
T toNumber (std::string text) {
    std::stringstream ss(text);
    T result;
    ss >> result;
    return result;
}

template <typename T>
std::string toString(T number) {
    std::stringstream out;
    out << number;
    return out.str();
}

class deleter {
    public:
        template <typename T>
        void operator()(const T& x) const {
            delete x;
        }
};

} //end of eddic

#endif
