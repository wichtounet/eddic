//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "Context.hpp"
#include "Variable.hpp"
#include "VisitorUtils.hpp"

#include "ast/GetTypeVisitor.hpp"
#include "ast/TypeTransformer.hpp"
#include "ast/Value.hpp"

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

Type ast::GetTypeVisitor::operator()(const ast::Cast& cast) const {
   return visit(ast::TypeTransformer(), cast.Content->type); 
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

Type ast::GetTypeVisitor::operator()(const ast::StructValue& struct_) const {
    auto var = (*struct_.Content->context)[struct_.Content->variableName];
    auto struct_name = var->type().type();
    auto struct_type = symbols.get_struct(struct_name);

    auto& members = struct_.Content->memberNames;
    for(std::size_t i = 0; i < members.size(); ++i){
        auto member_type = (*struct_type)[members[i]]->type;

        if(i == members.size() - 1){
            return member_type;
        } else {
            struct_name = member_type.type();
            struct_type = symbols.get_struct(struct_name);
        }
    }

    ASSERT_PATH_NOT_TAKEN("Problem with the type of members in nested struct values")
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
    return call.Content->function->returnType;
}
