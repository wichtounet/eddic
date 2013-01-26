/*******************************************************************************
 *
 * Copyright (c) 2008, 2009 Michael Schulze <mschulze@ivs.cs.uni-magdeburg.de>
 * All rights reserved.
 *
 *    Redistribution and use in source and binary forms, with or without
 *    modification, are permitted provided that the following conditions
 *    are met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in
 *      the documentation and/or other materials provided with the
 *      distribution.
 *
 *    * Neither the name of the copyright holders nor the names of
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 *    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * $Id: OutputStream.h 1 2010-01-11 14:24:28Z perch $
 *
 ******************************************************************************/

#ifndef __OutputStream_h__
#define __OutputStream_h__

#include "logging/Logger.h"

namespace logging {

    /*! \brief The %OutputStream provides different overloading of operator,
     *         enabling the output of different types in an uniform way.
     *
     *         The behaviour of the %OutputStream is the same as known from the
     *         standard streams. The %OutputStream provides manipulators too,
     *         and is very easy to extend, enabling the adaptation by the user.
     *
     * \tparam Base is a type used for the output and usually it is an output
     *         %device.
     */
    template< typename Base>
    class OutputStream : public Base {
        public:
            /*!
             * \brief Operator for catching manipulators
             * \param m is the manipulator that is feed to the output stream
             * \return %OutputStream& allows for chaining of operators
             **/
            OutputStream& operator << (const ::logging::log::Manipulator m) {
                Base::operator<<(static_cast<char>(m));
                return *this;
            }

            /*! \brief enable calling of manipulator functions
             *
             * \param f is a function that is called with the %OutputStream&
             *        as parameter.
             * \return %OutputStream& allows for chaining of operators
             */
            OutputStream& operator << (OutputStream& (*f) (OutputStream&)) {
                return f(*this);
            }

            template<typename T>
            OutputStream& operator << (const T& value){
                Base::operator<<(value);
                return *this;
            }
    };

} /* logging */

#endif
