//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_VALUE_H
#define AST_VALUE_H

#include "variant.hpp"

#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>

#include "ast/values_def.hpp"
#include "ast/Integer.hpp"
#include "ast/IntegerSuffix.hpp"
#include "ast/Float.hpp"
#include "ast/CharLiteral.hpp"
#include "ast/Literal.hpp"
#include "ast/VariableValue.hpp"
#include "ast/True.hpp"
#include "ast/False.hpp"
#include "ast/Boolean.hpp"
#include "ast/Null.hpp"

namespace eddic {

namespace ast {

typedef x3::variant<
            Integer,
            IntegerSuffix,
            Float,
            Literal,
            CharLiteral,
            VariableValue,
            Boolean,
            Null,
            x3::forward_ast<Expression>,
            x3::forward_ast<FunctionCall>,
            x3::forward_ast<Cast>,
            x3::forward_ast<BuiltinOperator>,
            x3::forward_ast<Assignment>,
            x3::forward_ast<PrefixOperation>,
            x3::forward_ast<Ternary>,
            x3::forward_ast<New>,
            x3::forward_ast<NewArray>
        > Value;

} //end of ast

} //end of eddic

#include "ast/Cast.hpp"
#include "ast/New.hpp"

#include "ast/Assignment.hpp"
#include "ast/Ternary.hpp"
#include "ast/Expression.hpp"
#include "ast/FunctionCall.hpp"
#include "ast/BuiltinOperator.hpp"
#include "ast/NewArray.hpp"
#include "ast/PrefixOperation.hpp"

X3_FORWARD_AST(eddic::ast::Expression)
X3_FORWARD_AST(eddic::ast::FunctionCall)
X3_FORWARD_AST(eddic::ast::Cast)
X3_FORWARD_AST(eddic::ast::BuiltinOperator)
X3_FORWARD_AST(eddic::ast::Assignment)
X3_FORWARD_AST(eddic::ast::PrefixOperation)
X3_FORWARD_AST(eddic::ast::Ternary)
X3_FORWARD_AST(eddic::ast::New)
X3_FORWARD_AST(eddic::ast::NewArray)

#endif
