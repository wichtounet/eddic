//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "tac/IsSingleArgumentVisitor.hpp"

#include "ast/Value.hpp"

#include "Types.hpp"
#include "VisitorUtils.hpp"

using namespace eddic;

ASSIGN_INSIDE_CONST_CONST(tac::IsSingleArgumentVisitor, ast::VariableValue, true)
ASSIGN_INSIDE_CONST_CONST(tac::IsSingleArgumentVisitor, ast::Integer, true)
ASSIGN_INSIDE_CONST_CONST(tac::IsSingleArgumentVisitor, ast::Float, true)
ASSIGN_INSIDE_CONST_CONST(tac::IsSingleArgumentVisitor, ast::True, true)
ASSIGN_INSIDE_CONST_CONST(tac::IsSingleArgumentVisitor, ast::False, true)

ASSIGN_INSIDE_CONST_CONST(tac::IsSingleArgumentVisitor, ast::Litteral, false)
ASSIGN_INSIDE_CONST_CONST(tac::IsSingleArgumentVisitor, ast::ArrayValue, false)
ASSIGN_INSIDE_CONST_CONST(tac::IsSingleArgumentVisitor, ast::ComposedValue, false)
ASSIGN_INSIDE_CONST_CONST(tac::IsSingleArgumentVisitor, ast::Minus, false)
ASSIGN_INSIDE_CONST_CONST(tac::IsSingleArgumentVisitor, ast::Plus, false)
ASSIGN_INSIDE_CONST_CONST(tac::IsSingleArgumentVisitor, ast::BuiltinOperator, false)
ASSIGN_INSIDE_CONST_CONST(tac::IsSingleArgumentVisitor, ast::Assignment, false)

//A call to a function returning an int is single argument
bool tac::IsSingleArgumentVisitor::operator()(const ast::FunctionCall& call) const {
    Type type = call.Content->function->returnType;

    return type == BaseType::INT || type == BaseType::BOOL;
}
