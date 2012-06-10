//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "SymbolTable.hpp"
#include "Type.hpp"

#include "ast/TypeTransformer.hpp"

using namespace eddic;

std::shared_ptr<Type> ast::TypeTransformer::operator()(ast::SimpleType& type) const {
    return new_type(type.type);
}

std::shared_ptr<Type> ast::TypeTransformer::operator()(ast::ArrayType& type) const {
    return new_array_type(new_type(type.type));
}
