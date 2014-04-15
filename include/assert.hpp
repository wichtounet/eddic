//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef EDDIC_ASSERT_H
#define EDDIC_ASSERT_H

#include <cassert>

#include <boost/assert.hpp>

/*!
 * \def eddic_assert(condition, message)
 * \brief Verify that the condition is true. If not, fails and display the specified message. 
 * \param condition The condition that have to be true
 * \param message The message to be printed if the assertion is not verified. 
 */
#define eddic_assert(condition, message) BOOST_ASSERT_MSG(condition, message);

#ifdef __GNUC__

/*!
 * \def eddic_unreachable(message)
 * \brief Assert that this path is not taken. If it is taken, fails and display the specified message. 
 * \param message The message to be printed if the assertion is not verified. 
 */
#define eddic_unreachable(message) BOOST_ASSERT_MSG(false, message); assert(false); __builtin_unreachable();

#endif

#ifdef __clang__

#if __has_builtin(__builtin_unreachable)
    /*!
     * \def eddic_unreachable(message)
     * \brief Assert that this path is not taken. If it is taken, fails and display the specified message. 
     * \param message The message to be printed if the assertion is not verified. 
     */
    #define eddic_unreachable(message) BOOST_ASSERT_MSG(false, message); assert(false); __builtin_unreachable();
#else
    /*!
     * \def eddic_unreachable(message)
     * \brief Assert that this path is not taken. If it is taken, fails and display the specified message. 
     * \param message The message to be printed if the assertion is not verified. 
     */
    #define eddic_unreachable(message) BOOST_ASSERT_MSG(false, message); assert(false);
#endif

#endif

#endif
