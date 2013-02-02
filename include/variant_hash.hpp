//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BOOST_HASH_VARIANT_FUNCTION_HPP
#define BOOST_HASH_VARIANT_FUNCTION_HPP

#include <boost/functional/hash.hpp>

#include "variant.hpp"

namespace boost {

namespace detail { namespace variant {
    struct variant_hasher: public boost::static_visitor<std::size_t> {
        template <class T>
        std::size_t operator()(T const& val) const {
            std::hash<T> hasher;
            return hasher(val);
        }
    };
}}

} //end of boost

namespace std {
    template<BOOST_VARIANT_ENUM_PARAMS(typename T)>
    class hash<boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)>> {
        public:
            std::size_t operator()(const boost::variant< BOOST_VARIANT_ENUM_PARAMS(T)>& val) const {
                std::size_t seed = boost::apply_visitor(boost::detail::variant::variant_hasher(), val);
                boost::hash_combine(seed, val.which());
                return seed;
            }
    };
}

#endif
