//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "SemanticalException.hpp"

using namespace eddic;

SemanticalException::SemanticalException(const std::string& message) : m_message(message) {}
SemanticalException::SemanticalException(const std::string& message, eddic::ast::Position& position) : m_message(message), m_position(position) {}

SemanticalException::~SemanticalException() throw() {}

const char* SemanticalException::what() const throw() {
    return m_message.c_str();
}
        
const std::string& SemanticalException::message() const {
    return m_message;
}

boost::optional<eddic::ast::Position> SemanticalException::position() const {
    return m_position;
}
