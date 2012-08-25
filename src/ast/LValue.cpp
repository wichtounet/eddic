//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License: Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"

#include "ast/Value.hpp"
#include "ast/LValue.hpp"

using namespace eddic;

ast::LValue eddic::ast::to_left_value(ast::Value left_value){
    if(auto* ptr = boost::get<ast::VariableValue>(&left_value)){
        return *ptr;
    } else if(auto* ptr = boost::get<ast::ArrayValue>(&left_value)){
        return *ptr;
    } else if(auto* ptr = boost::get<ast::MemberValue>(&left_value)){
        return *ptr;
    } else if(auto* ptr = boost::get<ast::DereferenceValue>(&left_value)){
        return *ptr;
    } else {
        ASSERT_PATH_NOT_TAKEN("Not a left value");
    }
}
