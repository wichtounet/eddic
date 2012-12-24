//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TERMINATION_EXCEPTION_H
#define TERMINATION_EXCEPTION_H

namespace eddic {

/*!
 * \struct TerminationException
 * \brief An exception indicating that an other exception has been found and already handled and that
 * the compilation has to be terminated. 
 */
class TerminationException: public std::exception {
    //Nothing
};

} //end of eddic

#endif
