//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LABELS_H
#define LABELS_H

#include "Utils.hpp"

namespace eddic {

std::string newLabel();

template<typename T>
std::string label(const std::string& prefix, T value){
    return prefix + toString(value);
}

} //end of eddic

#endif
