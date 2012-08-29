//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License: Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ast/SimpleType.hpp"

using namespace eddic;

bool ast::operator==(const ast::SimpleType& a, const ast::SimpleType& b){
    return a.const_ == b.const_ && a.type == b.type;
}
