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

ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::Litteral, newSimpleType(BaseType::STRING))

ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::Integer, newSimpleType(BaseType::INT))
ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::IntegerSuffix, newSimpleType(BaseType::FLOAT)) //For now, there is only a float (f) suffix
ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::BuiltinOperator, newSimpleType(BaseType::INT)) //At this time, all the builtin operators return an int

ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::Float, newSimpleType(BaseType::FLOAT))

ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::False, newSimpleType(BaseType::BOOL))
ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::True, newSimpleType(BaseType::BOOL))

Type ast::GetTypeVisitor::operator()(const ast::Minus& minus) const {
   return visit(*this, minus.Content->value); 
}

Type ast::GetTypeVisitor::operator()(const ast::Plus& minus) const {
   return visit(*this, minus.Content->value); 
}

Type ast::GetTypeVisitor::operator()(const ast::SuffixOperation& operation) const {
   return operation.Content->variable->type(); 
}

Type ast::GetTypeVisitor::operator()(const ast::PrefixOperation& operation) const {
   return operation.Content->variable->type(); 
}

Type ast::GetTypeVisitor::operator()(const ast::VariableValue& variable) const {
    return variable.Content->context->getVariable(variable.Content->variableName)->type();
}

Type ast::GetTypeVisitor::operator()(const ast::Assignment& assign) const {
    return assign.Content->context->getVariable(assign.Content->variableName)->type();
}

Type ast::GetTypeVisitor::operator()(const ast::ArrayValue& array) const {
    return newSimpleType(array.Content->context->getVariable(array.Content->arrayName)->type().base());
}

Type ast::GetTypeVisitor::operator()(const ast::Expression& value) const {
    auto op = value.Content->operations[0].get<0>();

    if(op == ast::Operator::AND || op == ast::Operator::OR){
        return newSimpleType(BaseType::BOOL);
    } else if(op >= ast::Operator::EQUALS && op <= ast::Operator::GREATER_EQUALS){
        return newSimpleType(BaseType::BOOL);
    } else {
        //No need to recurse into operations because type are enforced in the check variables phase
        return visit(*this, value.Content->first);
    }
}

Type ast::GetTypeVisitor::operator()(const ast::FunctionCall& call) const {
    std::string name = call.Content->functionName;

    return call.Content->function->returnType;
}
