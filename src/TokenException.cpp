//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "TokenException.hpp"

#include <sstream>

using namespace eddic;

using std::stringstream;

TokenException::TokenException(const std::string& message, std::shared_ptr<Token> token) : CompilerException(message), m_token(token) {}
TokenException::~TokenException() throw() {}

const char* TokenException::what() const throw() {
    stringstream value;

    value << m_message;
    value << std::endl;
    value << "\tline:" << m_token->line();
    value << " col:" << m_token->col();

    return value.str().c_str();
}
