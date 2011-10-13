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

#include "SpiritToken.hpp"

namespace eddic {

class CompilerException: public std::exception {
    protected:
        std::string m_message;
        const Tok& m_token;

    public:
        CompilerException(const std::string& message, const Tok& token);
        ~CompilerException() throw();

        virtual const char* what() const throw();
};

} //end of eddic

#endif
