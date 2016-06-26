//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef EDDIC_BOOST_CFG_H
#define EDDIC_BOOST_CFG_H

#include <exception>

namespace boost {

inline void throw_exception(const std::exception&){}

} // namespace boost

#define EDDIC_BOOST_LIMIT_SIZE 30
#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#define BOOST_MPL_LIMIT_VECTOR_SIZE EDDIC_BOOST_LIMIT_SIZE
#define BOOST_MPL_LIMIT_LIST_SIZE EDDIC_BOOST_LIMIT_SIZE
#define BOOST_SPIRIT_USE_PHOENIX_V3

//#define BOOST_SPIRIT_X3_DEBUG
#define BOOST_SPIRIT_X3_NO_RTTI

#endif
