//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "CompilerException.hpp"

#include <sstream>

using namespace eddic;

using std::stringstream;

const char* CompilerException::what() const throw() {
    stringstream value;

    value << m_message;

    if(m_token){
        value << std::endl;
        value << "\tline:" << m_token->line();
        value << " col:" << m_token->col();
    }

    return value.str().c_str();
}

const char* TokenException::what() const throw() {
    stringstream value;

    value << m_message;
    value << std::endl;
    value << "\tline:" << m_token->line();
    value << " col:" << m_token->col();

    return value.str().c_str();
}
