//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the Boost Software License: Version 1.0.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "ast/ArrayType.hpp"

using namespace eddic;

bool ast::operator==(const ast::ArrayType& a, const ast::ArrayType& b){
    return a.type.get() == b.type.get();
}

std::ostream& ast::operator<<(std::ostream& out, const ast::ArrayType& type){
    return out << "Array Type " << type.type;
}
