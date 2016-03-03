//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_SOURCE_FILE_DEF_H
#define AST_SOURCE_FILE_DEF_H

#include "ast/Deferred.hpp"

namespace eddic {

namespace ast {

struct ASTSourceFile;
typedef Deferred<ASTSourceFile> SourceFile;

struct ast_struct_definition;
typedef Deferred<ast_struct_definition> struct_definition;

//Functions

struct ASTFunctionDeclaration;
typedef Deferred<ASTFunctionDeclaration> FunctionDeclaration;

struct ASTConstructor;
typedef Deferred<ASTConstructor> Constructor;

struct ASTDestructor;
typedef Deferred<ASTDestructor> Destructor;

struct ASTTemplateFunctionDeclaration;
typedef Deferred<ASTTemplateFunctionDeclaration> TemplateFunctionDeclaration;

//Instructions

struct FunctionCall;

struct ASTExpression;
typedef Deferred<ASTExpression> Expression;

} //end of ast

} //end of eddic

#endif
