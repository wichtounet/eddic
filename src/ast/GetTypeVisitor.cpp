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
#include "Types.hpp"
#include "Type.hpp"

#include "ast/GetTypeVisitor.hpp"
#include "ast/TypeTransformer.hpp"
#include "ast/Value.hpp"

using namespace eddic;

ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::Litteral, STRING)

ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::Integer, INT)
ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::BuiltinOperator, INT) //At this time, all the builtin operators return an int
ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::IntegerSuffix, FLOAT) //For now, there is only a float (f) suffix

ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::Float, FLOAT)

ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::False, BOOL)
ASSIGN_INSIDE_CONST_CONST(ast::GetTypeVisitor, ast::True, BOOL)

std::shared_ptr<Type> ast::GetTypeVisitor::operator()(const ast::Minus& minus) const {
   return visit(*this, minus.Content->value); 
}

std::shared_ptr<Type> ast::GetTypeVisitor::operator()(const ast::Plus& minus) const {
   return visit(*this, minus.Content->value); 
}

std::shared_ptr<Type> ast::GetTypeVisitor::operator()(const ast::Cast& cast) const {
   return visit(ast::TypeTransformer(), cast.Content->type); 
}

std::shared_ptr<Type> ast::GetTypeVisitor::operator()(const ast::SuffixOperation& operation) const {
   return operation.Content->variable->type(); 
}

std::shared_ptr<Type> ast::GetTypeVisitor::operator()(const ast::PrefixOperation& operation) const {
   return operation.Content->variable->type(); 
}

std::shared_ptr<Type> ast::GetTypeVisitor::operator()(const ast::VariableValue& variable) const {
    auto var = (*variable.Content->context)[variable.Content->variableName];

    if(variable.Content->memberNames.empty()){
        return var->type();
    } else {
        auto struct_name = var->type()->type();
        auto struct_type = symbols.get_struct(struct_name);

        auto& members = variable.Content->memberNames;
        for(std::size_t i = 0; i < members.size(); ++i){
            auto member_type = (*struct_type)[members[i]]->type;

            if(i == members.size() - 1){
                return member_type;
            } else {
                struct_name = member_type->type();
                struct_type = symbols.get_struct(struct_name);
            }
        }

        ASSERT_PATH_NOT_TAKEN("Problem with the type of members in nested struct values")
    }
}

std::shared_ptr<Type> ast::GetTypeVisitor::operator()(const ast::Assignment& assign) const {
    return assign.Content->context->getVariable(assign.Content->variableName)->type();
}

std::shared_ptr<Type> ast::GetTypeVisitor::operator()(const ast::ArrayValue& array) const {
    return array.Content->context->getVariable(array.Content->arrayName)->type()->element_type();
}

std::shared_ptr<Type> ast::GetTypeVisitor::operator()(const ast::Expression& value) const {
    auto op = value.Content->operations[0].get<0>();

    if(op == ast::Operator::AND || op == ast::Operator::OR){
        return BOOL;
    } else if(op >= ast::Operator::EQUALS && op <= ast::Operator::GREATER_EQUALS){
        return BOOL;
    } else {
        //No need to recurse into operations because type are enforced in the check variables phase
        return visit(*this, value.Content->first);
    }
}

std::shared_ptr<Type> ast::GetTypeVisitor::operator()(const ast::FunctionCall& call) const {
    return call.Content->function->returnType;
}
