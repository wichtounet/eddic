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
    LOCAL
};

template<typename T>
struct pass_traits {
    
};

} //end of mtac

} //end of eddic

#endif
