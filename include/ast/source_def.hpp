//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_SOURCE_FILE_DEF_H
#define AST_SOURCE_FILE_DEF_H

#include "ast/Deferred.hpp"

namespace eddic {

namespace ast {

struct ASTSourceFile;
typedef Deferred<ASTSourceFile> SourceFile;

//Structures

struct ASTStruct;
typedef Deferred<ASTStruct> Struct;

struct ASTTemplateStruct;
typedef Deferred<ASTTemplateStruct> TemplateStruct;

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

struct ASTFunctionCall;
typedef Deferred<ASTFunctionCall> FunctionCall;

struct ASTMemberFunctionCall;
typedef Deferred<ASTMemberFunctionCall> MemberFunctionCall;

} //end of ast

} //end of eddic

#endif
