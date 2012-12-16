//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_INSTRUCTION_H
#define AST_INSTRUCTION_H

#include "variant.hpp"

#include "ast/Deferred.hpp"
#include "ast/Swap.hpp"
#include "ast/FunctionCall.hpp"
#include "ast/Expression.hpp"
#include "ast/Return.hpp"
#include "ast/Delete.hpp"
#include "ast/VariableDeclaration.hpp"
#include "ast/StructDeclaration.hpp"
#include "ast/ArrayDeclaration.hpp"
#include "ast/PrefixOperation.hpp"

namespace eddic {

namespace ast {

struct ASTWhile;
typedef Deferred<ASTWhile> While; 

struct ASTDoWhile;
typedef Deferred<ASTDoWhile> DoWhile; 

struct ASTFor;
typedef Deferred<ASTFor> For; 

struct ASTIf;
typedef Deferred<ASTIf> If;

struct ASTForeach;
typedef Deferred<ASTForeach> Foreach;

struct ASTForeachIn;
typedef Deferred<ASTForeachIn> ForeachIn;

struct ASTSwitch;
typedef Deferred<ASTSwitch> Switch;

struct ASTDelete;
typedef Deferred<ASTDelete> Delete;

typedef boost::variant<
            FunctionCall, 
            Expression,
            Swap, 
            VariableDeclaration,
            StructDeclaration,
            ArrayDeclaration,
            Assignment, 
            Return,
            If, 
            While, 
            DoWhile, 
            Foreach,
            ForeachIn, 
            PrefixOperation,
            Delete,
            Switch,
            For>
        Instruction;

} //end of ast

} //end of eddic

#include "ast/Assignment.hpp"
#include "ast/If.hpp"
#include "ast/While.hpp"
#include "ast/DoWhile.hpp"
#include "ast/For.hpp"
#include "ast/Foreach.hpp"
#include "ast/ForeachIn.hpp"
#include "ast/Switch.hpp"

#endif
