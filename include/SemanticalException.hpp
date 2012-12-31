//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SEMANTICAL_EXCEPTION_H
#define SEMANTICAL_EXCEPTION_H

#include <string>

#include <boost/optional.hpp>

#include "ast/Position.hpp"

namespace eddic {

/*!
 * \struct SemanticalException
 * \brief An exception occuring after the AST has been correctly constructed. 
 */
class SemanticalException: public std::exception {
    protected:
        std::string m_message;
        boost::optional<eddic::ast::Position> m_position;

    public:
        SemanticalException(const std::string& message);
        SemanticalException(const std::string& message, const eddic::ast::Position& position);

        ~SemanticalException() throw();

        /*!
         * Return the error message. 
         * \return The error message. 
         */
        const std::string& message() const;
        
        /*!
         * Return the position in the source file where this error comes from. This value is optional. 
         * \return The position of the error. 
         */
        boost::optional<eddic::ast::Position> position() const;

        virtual const char* what() const throw();
};

void output_exception(const SemanticalException& e);

} //end of eddic

#endif
