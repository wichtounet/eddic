//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_VALUE_H
#define AST_VALUE_H

#include "variant.hpp"

#include "ast/values_def.hpp"
#include "ast/Integer.hpp"
#include "ast/IntegerSuffix.hpp"
#include "ast/Float.hpp"
#include "ast/CharLiteral.hpp"
#include "ast/Literal.hpp"
#include "ast/VariableValue.hpp"
#include "ast/True.hpp"
#include "ast/False.hpp"
#include "ast/Null.hpp"

namespace eddic {

namespace ast {

typedef boost::variant<
            Integer,
            IntegerSuffix,
            Float,
            Literal,
            CharLiteral,
            VariableValue,
            Expression,
            Null,
            True,
            False,
            FunctionCall,
            boost::recursive_wrapper<Cast>,
            BuiltinOperator,
            Assignment,
            PrefixOperation,
            Ternary,
            New,
            NewArray
        > Value;

} //end of ast

} //end of eddic

#include "ast/Cast.hpp"

#include "ast/Assignment.hpp"
#include "ast/Ternary.hpp"
#include "ast/Expression.hpp"
#include "ast/FunctionCall.hpp"
#include "ast/BuiltinOperator.hpp"
#include "ast/New.hpp"
#include "ast/NewArray.hpp"
#include "ast/PrefixOperation.hpp"

#endif
