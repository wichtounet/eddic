//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License: Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ast/ArrayType.hpp"

using namespace eddic;

bool ast::operator==(const ast::ArrayType& a, const ast::ArrayType& b){
    return a.type == b.type;
}
