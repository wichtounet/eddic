//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>

#include "Warnings.hpp"

void eddic::warn(const std::string& warning){
    std::cout << "warning: " << warning << std::endl;
}

void eddic::warn(const eddic::ast::Position& position, const std::string& warning){
    std::cout << position.file << ":" << position.line << ": warning: " << warning << std::endl;
}
