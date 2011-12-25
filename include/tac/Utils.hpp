//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_UTILS_H
#define TAC_UTILS_H

#include <boost/variant.hpp>

namespace eddic {

namespace tac {

template<typename V, typename T>
bool equals(T& variant, V value){
    return boost::get<V>(&variant) && boost::get<V>(variant) == value;
}

} //end of tac

} //end of eddic

#endif
