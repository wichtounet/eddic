//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef SEMANTICAL_EXCEPTION_H
#define SEMANTICAL_EXCEPTION_H

#include <string>
#include <memory>

#include "GlobalContext.hpp"

namespace eddic {

/*!
 * \struct SemanticalException
 * \brief An exception occuring after the AST has been correctly constructed.
 */
class SemanticalException: public std::exception {
    protected:
        std::string m_message;

    public:
        SemanticalException(std::string message);

        ~SemanticalException() throw();

        /*!
         * Return the error message.
         * \return The error message.
         */
        const std::string& message() const;

        virtual const char* what() const throw();
};

//TODO Remove second parameter
void output_exception(const SemanticalException& e, std::shared_ptr<GlobalContext> context);

} //end of eddic

#endif
