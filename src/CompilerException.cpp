//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "CompilerException.hpp"

#include <sstream>

#include "SpiritLexer.hpp"

using namespace eddic;

using std::stringstream;

CompilerException::CompilerException(const std::string& message, const Tok& token) : m_message(message), m_token(token) {}
CompilerException::~CompilerException() throw() {}

const char* CompilerException::what() const throw() {
    stringstream value;

    value << m_message;

    if(m_token){
        value << std::endl;
        //value << "\tline:" << m_token->line();
        //value << " col:" << m_token->col();
    }

    return value.str().c_str();
}
