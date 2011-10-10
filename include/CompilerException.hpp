//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef COMPILER_EXCEPTION_H
#define COMPILER_EXCEPTION_H

#include <string>
#include <memory>

#include "Token.hpp"

namespace eddic {

class CompilerException: public std::exception {
    protected:
        std::string m_message;
        const std::shared_ptr<Token> m_token;

    public:
        CompilerException(const std::string& message);
        CompilerException(const std::string& message, const std::shared_ptr<Token> token);
        ~CompilerException() throw();

        virtual const char* what() const throw();
};

} //end of eddic

#endif
