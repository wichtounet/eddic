//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef EDDIC_BOOST_VARIANT_H
#define EDDIC_BOOST_VARIANT_H

#define BOOST_NO_RTTI
#define BOOST_NO_TYPEID

#include "boost_cfg.hpp"

#include <boost/variant.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>

//For those who need variant with more than 20 template parameters
#include <boost/mpl/vector.hpp>

#include "smart_get.hpp"

namespace x3 = boost::spirit::x3;

namespace eddic {

namespace ast {

template<typename V1, typename... V2>
std::ostream& operator<<(std::ostream& os, const boost::spirit::x3::variant<V1, V2...>& v1){
    return os << v1.get();
}

template<typename... V1, typename... V2>
bool operator==(const boost::spirit::x3::variant<V1...>& v1, const boost::spirit::x3::variant<V2...>& v2){
    return v1.get() == v2.get();
}

template<typename... V1, typename... V2>
bool operator!=(const boost::spirit::x3::variant<V1...>& v1, const boost::spirit::x3::variant<V2...>& v2){
    return v1.get() != v2.get();
}


} //end of namespace ast

} //end of namespace eddic

#endif
