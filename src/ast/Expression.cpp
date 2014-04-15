//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the Boost Software License: Version 1.0.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "ast/Value.hpp"
#include "ast/Expression.hpp"

using namespace eddic;

bool ast::has_operation_value(const ast::Operation& op){
    return op.get<0>() != ast::Operator::DEC || op.get<0>() != ast::Operator::INC;
}
