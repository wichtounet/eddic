//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TOKEN_EXCEPTION_H
#define TOKEN_EXCEPTION_H

#include <string>
#include <memory>

#include "CompilerException.hpp"

namespace eddic {

class TokenException: public CompilerException {
    private:
        std::shared_ptr<Token> m_token;

    public:
        TokenException(const std::string& message, std::shared_ptr<Token> token);
        ~TokenException() throw();

        const char* what() const throw();
};

} //end of eddic

#endif
