//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
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

void eddic::output_exception(const SemanticalException& e, std::shared_ptr<GlobalContext> context){
    if(e.position()){
        const auto& position = *e.position();

        int current_line = 0;
        std::istringstream f(context->get_file_content(position.file));
        std::string line;
        while (std::getline(f, line)) {
            if(current_line == position.line){
                break;
            }

            ++current_line;
        }

        std::cout << position.file << ":" << position.line << ":" << " error: " << e.what() << std::endl;
        std::cout << line << std::endl;
        std::cout << std::setw(position.column) << " ^- here" << std::endl;
    } else {
        std::cout << e.what() << std::endl;
    }
}
