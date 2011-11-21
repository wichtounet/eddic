//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "TypeTransformer.hpp"

using namespace eddic;

Type TypeTransformer::operator()(ast::SimpleType& type) const {
    return Type(stringToBaseType(type.type));
}

Type TypeTransformer::operator()(ast::ArrayType& type) const {
    return Type(stringToBaseType(type.type), 0);//TODO Find a better way to declare a array type
}
