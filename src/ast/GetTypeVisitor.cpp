//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ast/GetTypeVisitor.hpp"
#include "ast/Value.hpp"

#include "Context.hpp"
#include "Variable.hpp"
#include "VisitorUtils.hpp"

using namespace eddic;

ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::Litteral, Type(BaseType::STRING, false))

ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::Integer, Type(BaseType::INT, false))
ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::Minus, Type(BaseType::INT, false))
ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::Plus, Type(BaseType::INT, false))
ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::SuffixOperation, Type(BaseType::INT, false))
ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::PrefixOperation, Type(BaseType::INT, false))
ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::BuiltinOperator, Type(BaseType::INT, false)) //At this time, all the builtin operators return an int

ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::False, Type(BaseType::BOOL, false))
ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::True, Type(BaseType::BOOL, false))

Type ast::GetTypeVisitor::operator()(const ast::VariableValue& variable) const {
    return variable.Content->context->getVariable(variable.Content->variableName)->type();
}

Type ast::GetTypeVisitor::operator()(const ast::Assignment& assign) const {
    return assign.Content->context->getVariable(assign.Content->variableName)->type();
}

Type ast::GetTypeVisitor::operator()(const ast::ArrayValue& array) const {
    return Type(array.Content->context->getVariable(array.Content->arrayName)->type().base(), false);
}

Type ast::GetTypeVisitor::operator()(const ast::ComposedValue& value) const {
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

Type ast::GetTypeVisitor::operator()(const ast::FunctionCall& call) const {
    std::string name = call.Content->functionName;

    assert(name != "println" && name != "print");

    return call.Content->function->returnType;
}
