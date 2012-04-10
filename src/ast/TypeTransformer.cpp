//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "Types.hpp"
#include "SymbolTable.hpp"

#include "ast/TypeTransformer.hpp"

using namespace eddic;

eddic::Type ast::TypeTransformer::operator()(ast::SimpleType& type) const {
    if(is_standard_type(type.type)){
        return newSimpleType(type.type);
    } else {
        ASSERT(symbols.struct_exists(type.type), "The struct does not exists");

        return new_custom_type(type.type);
    }
}

eddic::Type ast::TypeTransformer::operator()(ast::ArrayType& type) const {
    return newArrayType(type.type);
}
