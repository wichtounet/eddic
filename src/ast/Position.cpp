//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the Boost Software License: Version 1.0.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "ast/Position.hpp"

using namespace eddic;

std::ostream& ast::operator<< (std::ostream& stream, ast::Position){
    return stream << "Position";
}

bool ast::operator==(const ast::Position& a, const ast::Position& b){
    return a.file == b.file && a.line == b.line && a.column == b.column;
}

bool ast::operator!=(const ast::Position& a, const ast::Position& b){
    return !(a == b);
}
