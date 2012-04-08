//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_INSTRUCTION_H
#define AST_INSTRUCTION_H

#include <boost/variant/variant.hpp>

#include "ast/Deferred.hpp"
#include "ast/Swap.hpp"
#include "ast/Assignment.hpp"
#include "ast/CompoundAssignment.hpp"
#include "ast/FunctionCall.hpp"
#include "ast/ArrayAssignment.hpp"
#include "ast/StructAssignment.hpp"
#include "ast/Return.hpp"
#include "ast/VariableDeclaration.hpp"
#include "ast/ArrayDeclaration.hpp"
#include "ast/SuffixOperation.hpp"
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

typedef boost::variant<
            FunctionCall, 
            Swap, 
            VariableDeclaration,
            ArrayDeclaration,
            Assignment, 
            CompoundAssignment,
            ArrayAssignment, 
            StructAssignment,
            Return,
            If, 
            While, 
            DoWhile, 
            Foreach,
            ForeachIn, 
            SuffixOperation,
            PrefixOperation,
            For>
        Instruction;

} //end of ast

} //end of eddic

#include "ast/If.hpp"
#include "ast/While.hpp"
#include "ast/DoWhile.hpp"
#include "ast/For.hpp"
#include "ast/Foreach.hpp"
#include "ast/ForeachIn.hpp"

#endif
