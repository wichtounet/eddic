//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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
