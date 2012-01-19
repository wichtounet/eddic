//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "GetTypeVisitor.hpp"
#include "Context.hpp"
#include "Variable.hpp"
#include "VisitorUtils.hpp"

#include "ast/Value.hpp"

using namespace eddic;

ASSIGN_INSIDE_CONST_CONST(GetTypeVisitor, ast::Litteral, Type(BaseType::STRING, false))

ASSIGN_INSIDE_CONST_CONST(GetTypeVisitor, ast::Integer, Type(BaseType::INT, false))
ASSIGN_INSIDE_CONST_CONST(GetTypeVisitor, ast::Minus, Type(BaseType::INT, false))
ASSIGN_INSIDE_CONST_CONST(GetTypeVisitor, ast::Plus, Type(BaseType::INT, false))

ASSIGN_INSIDE_CONST_CONST(GetTypeVisitor, ast::False, Type(BaseType::BOOL, false))
ASSIGN_INSIDE_CONST_CONST(GetTypeVisitor, ast::True, Type(BaseType::BOOL, false))

Type GetTypeVisitor::operator()(const ast::VariableValue& variable) const {
    return variable.Content->context->getVariable(variable.Content->variableName)->type();
}

Type GetTypeVisitor::operator()(const ast::ArrayValue& array) const {
    return Type(array.Content->context->getVariable(array.Content->arrayName)->type().base(), false);
}

Type GetTypeVisitor::operator()(const ast::ComposedValue& value) const {
    auto op = value.Content->operations[0].get<0>();

    if(op == ast::Operator::AND || op == ast::Operator::OR){
        return Type(BaseType::BOOL, false);
    } else if(op >= ast::Operator::EQUALS && op <= ast::Operator::GREATER_EQUALS){
        return Type(BaseType::BOOL, false);
    } else {
        //No need to recurse into operations because type are enforced in the check variables phase
        return visit(*this, value.Content->first);
    }
}

Type GetTypeVisitor::operator()(const ast::FunctionCall& call) const {
    std::string name = call.Content->functionName;

    assert(name != "println" && name != "print");

    return call.Content->function->returnType;
}
