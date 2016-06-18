//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_INSTRUCTION_H
#define AST_INSTRUCTION_H

#include "variant.hpp"

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

struct While;
struct DoWhile;
struct For;
struct If;
struct Foreach;
struct ForeachIn;
struct Switch;
struct Delete;

typedef x3::variant<
            FunctionCall,
            Expression,
            VariableDeclaration,
            StructDeclaration,
            ArrayDeclaration,
            Assignment,
            Return,
            x3::forward_ast<If>,
            x3::forward_ast<While>,
            x3::forward_ast<DoWhile>,
            x3::forward_ast<Foreach>,
            x3::forward_ast<ForeachIn>,
            PrefixOperation,
            Delete,
            x3::forward_ast<Switch>,
            x3::forward_ast<For>>
        Instruction;

} //end of ast

} //end of eddic

#include "ast/Assignment.hpp"
#include "ast/If.hpp"
#include "ast/While.hpp"
#include "ast/DoWhile.hpp"
#include "ast/Foreach.hpp"
#include "ast/ForeachIn.hpp"
#include "ast/Switch.hpp"
#include "ast/For.hpp"

X3_FORWARD_AST(eddic::ast::If)
X3_FORWARD_AST(eddic::ast::While)
X3_FORWARD_AST(eddic::ast::DoWhile)
X3_FORWARD_AST(eddic::ast::Foreach)
X3_FORWARD_AST(eddic::ast::ForeachIn)
X3_FORWARD_AST(eddic::ast::Switch)
X3_FORWARD_AST(eddic::ast::For)

#endif
