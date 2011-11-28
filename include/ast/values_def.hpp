//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VALUES_DEF_H
#define VALUES_DEF_H

#include "ast/Deferred.hpp"

namespace eddic {

namespace ast {

struct Integer;
struct Litteral;

struct ASTFunctionCall;
typedef Deferred<ASTFunctionCall> FunctionCall;

struct ASTVariableValue;
typedef Deferred<ASTVariableValue> VariableValue;

struct ASTComposedValue;
typedef Deferred<ASTComposedValue> ComposedValue;

struct ASTArrayValue;
typedef Deferred<ASTArrayValue> ArrayValue;

} //end of ast

} //end of eddic

#endif
