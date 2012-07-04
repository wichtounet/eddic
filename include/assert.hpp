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

/*!
 * \def ASSERT(condition, message)
 * \brief Verify that the condition is true. If not, fails and display the specified message. 
 * \param condition The condition that have to be true
 * \param message The message to be printed if the assertion is not verified. 
 */
#define ASSERT(condition, message) BOOST_ASSERT_MSG(condition, message);

#ifdef __GNUC__

/*!
 * \def ASSERT_PATH_NOT_TAKEN(message)
 * \brief Assert that this path is not taken. If it is taken, fails and display the specified message. 
 * \param message The message to be printed if the assertion is not verified. 
 */
#define ASSERT_PATH_NOT_TAKEN(message) BOOST_ASSERT_MSG(false, message); assert(false); __builtin_unreachable();

#else

/*!
 * \def ASSERT_PATH_NOT_TAKEN(message)
 * \brief Assert that this path is not taken. If it is taken, fails and display the specified message. 
 * \param message The message to be printed if the assertion is not verified. 
 */
#define ASSERT_PATH_NOT_TAKEN(message) BOOST_ASSERT_MSG(false, message); assert(false);

#endif

#endif
