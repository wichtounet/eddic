//=======================================================================
// Copyright Baptiste Wicht 2011.
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
        SemanticalException(const std::string& message, eddic::ast::Position& position);

        ~SemanticalException() throw();

        const std::string& message();
        boost::optional<eddic::ast::Position> position();

        virtual const char* what() const throw();
};

} //end of eddic

#endif
