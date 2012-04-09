//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef EDDIC_ASSERT_H
#define EDDIC_ASSERT_H

#include <cassert>

#include <boost/assert.hpp>

#define ASSERT_PATH_NOT_TAKEN(message) BOOST_ASSERT_MSG(false, message); assert(false);

#endif
