//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <iomanip>

#include "SemanticalException.hpp"

using namespace eddic;

SemanticalException::SemanticalException(std::string message) : m_message(std::move(message)) {}
SemanticalException::SemanticalException(std::string message, eddic::ast::Position position) : m_message(std::move(message)), m_position(std::move(position)) {}

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

void eddic::output_exception(const SemanticalException& e){
    if(e.position()){
        auto& position = *e.position();

        std::cout << position.file << ":" << position.line << ":" << " error: " << e.what() << std::endl;
        std::cout << "TODO Find the line using the context" << std::endl;
        std::cout << std::setw(position.column) << " ^- here" << std::endl;
    } else {
        std::cout << e.what() << std::endl;
    }
}
