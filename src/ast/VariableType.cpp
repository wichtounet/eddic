//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License: Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"

#include "ast/VariableType.hpp"

using namespace eddic;
    
std::string ast::to_string(const ast::Type& type){
    if(auto* ptr = boost::get<ast::SimpleType>(&type)){
        return ptr->type;
    } else if(auto* ptr = boost::get<ast::ArrayType>(&type)){
        return ptr->type + "[]";
    } else if(auto* ptr = boost::get<ast::PointerType>(&type)){
        return ptr->type + "*";
    } else {
        ASSERT_PATH_NOT_TAKEN("Unhandled type");
    }
}
