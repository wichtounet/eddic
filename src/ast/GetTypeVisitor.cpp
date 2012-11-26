//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "Context.hpp"
#include "Variable.hpp"
#include "VisitorUtils.hpp"
#include "Type.hpp"
#include "GlobalContext.hpp"
#include "mangling.hpp"

#include "ast/GetTypeVisitor.hpp"
#include "ast/TypeTransformer.hpp"
#include "ast/Value.hpp"

using namespace eddic;

ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::Literal, STRING)
ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::CharLiteral, CHAR)

ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::Integer, INT)
ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::BuiltinOperator, INT) //At this time, all the builtin operators return an int
ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::IntegerSuffix, FLOAT) //For now, there is only a float (f) suffix

ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::Float, FLOAT)

ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::False, BOOL)
ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::True, BOOL)

std::shared_ptr<const Type> ast::GetTypeVisitor::operator()(const ast::Null& /*null*/) const {
    return new_pointer_type(INT);
}

std::shared_ptr<const Type> ast::GetTypeVisitor::operator()(const ast::New& value) const {
    return new_pointer_type(visit(ast::TypeTransformer(value.Content->context->global()), value.Content->type));
}

std::shared_ptr<const Type> ast::GetTypeVisitor::operator()(const ast::NewArray& value) const {
    return new_array_type(visit(ast::TypeTransformer(value.Content->context->global()), value.Content->type));
}

std::shared_ptr<const Type> ast::GetTypeVisitor::operator()(const ast::Ternary& ternary) const {
   return visit(*this, ternary.Content->true_value); 
}

std::shared_ptr<const Type> ast::GetTypeVisitor::operator()(const ast::Cast& cast) const {
   return visit(ast::TypeTransformer(cast.Content->context->global()), cast.Content->type); 
}

std::shared_ptr<const Type> ast::GetTypeVisitor::operator()(const ast::PrefixOperation& operation) const {
    auto type = visit(*this, operation.Content->left_value);

    if(operation.Content->op == ast::Operator::STAR){
        return type->data_type();
    } else if(operation.Content->op == ast::Operator::ADDRESS){
        return new_pointer_type(type);
    } else {
        return type; 
    }
}

std::shared_ptr<const Type> ast::GetTypeVisitor::operator()(const ast::VariableValue& variable) const {
    return variable.Content->var->type();
}

std::shared_ptr<const Type> ast::GetTypeVisitor::operator()(const ast::Assignment& assign) const {
    return visit(*this, assign.Content->left_value);
}

std::shared_ptr<const eddic::Type> ast::operation_type(const std::shared_ptr<const eddic::Type> left, std::shared_ptr<Context> context, const ast::Operation& operation){
    auto op = operation.get<0>();
    auto global_context = context->global();

    if(op == ast::Operator::AND || op == ast::Operator::OR){
        return BOOL;
    } else if(op >= ast::Operator::EQUALS && op <= ast::Operator::GREATER_EQUALS){
        return BOOL;
    } else if(op == ast::Operator::CALL){
        auto type = left;

        if(type->is_pointer()){
            type = type->data_type();
        }

        auto operation_value = boost::get<ast::CallOperationValue>(*operation.get<1>());
        auto function_name = mangle(operation_value.get<0>(), operation_value.get<2>(), type);

        return global_context->getFunction(function_name)->returnType;
    } else if(op == ast::Operator::BRACKET){
        if(left == STRING){
            return CHAR;
        } else {
            return left->data_type();
        }
    } else if(op == ast::Operator::DOT){
        auto type = left;

        if(type->is_pointer()){
            type = type->data_type();
        }

        auto struct_type = global_context->get_struct(type->mangle());
        auto member = boost::get<std::string>(*operation.get<1>());
        return (*struct_type)[member]->type;
    } else {
        //Other operators are not changing the type
        return left;
    }
}

std::shared_ptr<const Type> ast::GetTypeVisitor::operator()(const ast::Expression& value) const {
    auto type = visit(*this, value.Content->first);

    for(auto& operation : value.Content->operations){
        type = ast::operation_type(type, value.Content->context, operation);
    }

    return type;
}

std::shared_ptr<const Type> ast::GetTypeVisitor::operator()(const ast::FunctionCall& call) const {
    return call.Content->function->returnType;
}

std::shared_ptr<const Type> ast::GetTypeVisitor::operator()(const std::shared_ptr<Variable> value) const {
    return value->type();
}
