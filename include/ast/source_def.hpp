//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_SOURCE_FILE_DEF_H
#define AST_SOURCE_FILE_DEF_H

namespace eddic {

namespace ast {

struct SourceFile;
struct struct_definition;

//Functions

struct FunctionDeclaration;
struct Constructor;
struct Destructor;
struct TemplateFunctionDeclaration;

//Instructions

struct FunctionCall;
struct Expression;

} //end of ast

} //end of eddic

#endif
