//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_PASS_TRAITS_H
#define MTAC_PASS_TRAITS_H

#include <boost/utility.hpp>

#define STATIC_CONSTANT(type,name,value) BOOST_STATIC_CONSTANT(type, name = value)
#define STATIC_STRING(name,value) static inline constexpr const char* name(){ return value; }

namespace eddic {

namespace mtac {

enum class pass_type : unsigned int {
    IPA,
    IPA_SUB,
    LOCAL, 
    DATA_FLOW,
    BB,
    BB_TWO_PASS,
    CUSTOM
};

enum TODO {
    TODO_REMOVE_NOP = 1,
    TODO_INVALIDATE_CFG = 2
};

enum PROPERTY {
    PROPERTY_POOL = 1,
    PROPERTY_PLATFORM = 2,
    PROPERTY_CONFIGURATION = 4
};

template<typename T>
struct pass_traits {

};

} //end of mtac

} //end of eddic

#endif
