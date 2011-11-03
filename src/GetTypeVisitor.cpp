//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "GetTypeVisitor.hpp"

#include "ast/Value.hpp"

using namespace eddic;

Type GetTypeVisitor::operator()(ASTLitteral& litteral) const {
    return Type::STRING;
}

Type GetTypeVisitor::operator()(ASTInteger& litteral) const {
    return Type::INT;
}

Type GetTypeVisitor::operator()(ASTVariable& variable) const {
    //TODO RETURN 
    return Type::INT;
}

Type GetTypeVisitor::operator()(ASTComposedValue& value) const {
    //TODO RETURN
    return Type::INT;
}
