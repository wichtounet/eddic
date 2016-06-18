//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_INSTRUCTION_H
#define AST_INSTRUCTION_H

#include "variant.hpp"

#include "ast/Deferred.hpp"
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

struct ASTIf;
typedef Deferred<ASTIf> If;

struct Foreach;
struct ForeachIn;
struct Switch;
struct Delete;

typedef boost::variant<
            FunctionCall,
            Expression,
            VariableDeclaration,
            StructDeclaration,
            ArrayDeclaration,
            Assignment,
            Return,
            If,
            boost::recursive_wrapper<While>,
            boost::recursive_wrapper<DoWhile>,
            boost::recursive_wrapper<Foreach>,
            boost::recursive_wrapper<ForeachIn>,
            PrefixOperation,
            Delete,
            boost::recursive_wrapper<Switch>,
            boost::recursive_wrapper<For>>
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
