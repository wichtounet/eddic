//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License: Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ast/Value.hpp"
#include "ast/Expression.hpp"

using namespace eddic;

bool ast::has_operation_value(const ast::Operation& op){
    return op.get<0>() != ast::Operator::DEC || op.get<0>() != ast::Operator::INC;
}
