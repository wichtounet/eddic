//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License: Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"

#include "ast/Position.hpp"

using namespace eddic;

std::ostream& ast::operator<< (std::ostream& stream, ast::Position){
    return stream << "Position";
}

bool ast::operator==(const ast::Position& a, const ast::Position& b){
    return a.file == b.file && a.theLine == b.theLine && a.line == b.line && a.column == b.column;
}

bool ast::operator!=(const ast::Position& a, const ast::Position& b){
    return !(a == b);
}
