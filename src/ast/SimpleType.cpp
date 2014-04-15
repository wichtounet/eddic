//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the Boost Software License: Version 1.0.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "ast/SimpleType.hpp"

using namespace eddic;

bool ast::operator==(const ast::SimpleType& a, const ast::SimpleType& b){
    return a.const_ == b.const_ && a.type == b.type;
}

std::ostream& ast::operator<<(std::ostream& out, const ast::SimpleType& type){
    return out << "Simple Type " << type.type;
}
