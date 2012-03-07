//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "Warnings.hpp"

void eddic::warn(const std::string& warning){
    std::cout << "warning: " << warning << std::endl;
}

void eddic::warn(const eddic::ast::Position& position, const std::string& warning){
    std::cout << position.file << ":" << position.line << ": warning: " << warning << std::endl;
}
