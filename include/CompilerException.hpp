//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef COMPILER_EXCEPTION_H
#define COMPILER_EXCEPTION_H

#include <string>

#include "Token.hpp"

namespace eddic {

class CompilerException: public std::exception {
    protected:
        std::string m_message;

    public:
        CompilerException(const std::string& message) : m_message(message) {};
        ~CompilerException() throw() {};

        virtual const char* what() const throw();
};

class TokenException: public CompilerException {
    private:
        Token m_token;

    public:
        TokenException(const std::string& message, Token token) : CompilerException(message), m_token(token) {}
        ~TokenException() throw() {};

        const char* what() const throw();
};

} //end of eddic

#endif
