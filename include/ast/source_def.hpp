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

struct ASTStruct;
typedef Deferred<ASTStruct> Struct;

struct ASTTemplateStruct;
typedef Deferred<ASTTemplateStruct> TemplateStruct;

struct ASTFunctionDeclaration;
typedef Deferred<ASTFunctionDeclaration> FunctionDeclaration;

struct ASTTemplateFunctionDeclaration;
typedef Deferred<ASTTemplateFunctionDeclaration> TemplateFunctionDeclaration;

} //end of ast

} //end of eddic

#endif
