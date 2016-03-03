//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef VALUES_DEF_H
#define VALUES_DEF_H

#include "ast/Deferred.hpp"

namespace eddic {

namespace ast {

struct Integer;
struct IntegerSuffix;
struct Float;
struct Literal;
struct True;
struct False;
struct Boolean;
struct Null;
struct Cast;
struct New;
struct BuiltinOperator;
struct NewArray;
struct PrefixOperation;
struct Ternary;
struct VariableValue;
struct Assignment;
struct FunctionCall;

struct ASTExpression;
typedef Deferred<ASTExpression> Expression;

} //end of ast

} //end of eddic

#endif
