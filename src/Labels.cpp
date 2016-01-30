//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "Labels.hpp"
#include "Utils.hpp"

__thread int currentLabel = 0;

std::string eddic::newLabel(){
   return "L" + toString(currentLabel++); 
}

void eddic::resetNumbering(){
    currentLabel = 0;
}
