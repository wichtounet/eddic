//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
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
struct IntegerSuffix;
struct Float;
struct Literal;
struct True;
struct False;
struct Null;

struct ASTFunctionCall;
typedef Deferred<ASTFunctionCall> FunctionCall;

struct ASTCast;
typedef Deferred<ASTCast> Cast;

struct ASTBuiltinOperator;
typedef Deferred<ASTBuiltinOperator> BuiltinOperator;

struct ASTAssignment;
typedef Deferred<ASTAssignment> Assignment;

struct ASTTernary;
typedef Deferred<ASTTernary> Ternary;

struct VariableValue;

struct ASTExpression;
typedef Deferred<ASTExpression> Expression;

struct ASTNew;
typedef Deferred<ASTNew> New;

struct ASTNewArray;
typedef Deferred<ASTNewArray> NewArray;

struct ASTPrefixOperation;
typedef Deferred<ASTPrefixOperation> PrefixOperation;

} //end of ast

} //end of eddic

#endif
