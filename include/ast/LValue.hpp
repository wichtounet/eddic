//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_LVALUE_H
#define AST_LVALUE_H

#include <boost/variant/variant.hpp>

#include "ast/ArrayValue.hpp"
#include "ast/VariableValue.hpp"

namespace eddic {

namespace ast {

struct ASTDereferenceValue;
typedef Deferred<ASTDereferenceValue> DereferenceValue;

typedef boost::variant<
            VariableValue, 
            ArrayValue,
            DereferenceValue
        > LValue;

ast::LValue to_left_value(ast::Value left_value);

} //end of ast

} //end of eddic

#include "ast/DereferenceValue.hpp"

#endif
