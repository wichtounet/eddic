//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <sstream>
#include <iomanip>

#include "SemanticalException.hpp"

using namespace eddic;

SemanticalException::SemanticalException(std::string message) : m_message(std::move(message)) {}

SemanticalException::~SemanticalException() throw() {}

const char* SemanticalException::what() const throw() {
    return m_message.c_str();
}

const std::string& SemanticalException::message() const {
    return m_message;
}

void eddic::output_exception(const SemanticalException& e, std::shared_ptr<GlobalContext>){
    std::cout << e.what() << std::endl;
}
