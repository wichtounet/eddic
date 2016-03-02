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

typedef boost::variant<
            Integer,
            IntegerSuffix,
            Float,
            Literal,
            CharLiteral,
            VariableValue,
            Expression,
            Null,
            Boolean,
            FunctionCall,
            boost::recursive_wrapper<Cast>,
            boost::recursive_wrapper<BuiltinOperator>,
            Assignment,
            boost::recursive_wrapper<PrefixOperation>,
            boost::recursive_wrapper<Ternary>,
            boost::recursive_wrapper<New>,
            boost::recursive_wrapper<NewArray>
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

#endif
