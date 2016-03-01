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

struct ASTFunctionCall;
typedef Deferred<ASTFunctionCall> FunctionCall;

struct ASTBuiltinOperator;
typedef Deferred<ASTBuiltinOperator> BuiltinOperator;

struct ASTAssignment;
typedef Deferred<ASTAssignment> Assignment;

struct ASTTernary;
typedef Deferred<ASTTernary> Ternary;

struct VariableValue;

struct ASTExpression;
typedef Deferred<ASTExpression> Expression;

struct ASTNewArray;
typedef Deferred<ASTNewArray> NewArray;

struct ASTPrefixOperation;
typedef Deferred<ASTPrefixOperation> PrefixOperation;

} //end of ast

} //end of eddic

#endif
