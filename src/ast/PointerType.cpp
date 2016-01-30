//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the Boost Software License: Version 1.0.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "ast/PointerType.hpp"

using namespace eddic;

bool ast::operator==(const ast::PointerType& a, const ast::PointerType& b){
    return a.type.get() == b.type.get();
}

std::ostream& ast::operator<<(std::ostream& out, const ast::PointerType& type){
    return out << "Pointer Type " << type.type.get();
}
