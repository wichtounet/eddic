//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef LABELS_H
#define LABELS_H

#include "Utils.hpp"

namespace eddic {

std::string newLabel();
void resetNumbering();

template<typename T>
std::string label(const std::string& prefix, T value){
    return prefix + toString(value);
}

} //end of eddic

#endif
