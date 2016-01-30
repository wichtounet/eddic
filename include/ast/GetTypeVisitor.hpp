//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef GET_TYPE_VISITOR_H
#define GET_TYPE_VISITOR_H

#include <memory>

#include "variant.hpp"

#include "ast/values_def.hpp"

namespace eddic {

class Variable;
class Type;

namespace ast {

/*!
 * \class Getconst eddic::TypeVisitor
 * \brief Static Visitor to get the const eddic::Type of a Value from the AST. 
 */
struct GetTypeVisitor : public boost::static_visitor<std::shared_ptr<const eddic::Type>> {
    std::shared_ptr<const eddic::Type> operator()(const ast::Assignment& assign) const; 
    std::shared_ptr<const eddic::Type> operator()(const ast::Ternary& assign) const; 
    std::shared_ptr<const eddic::Type> operator()(const ast::Literal& literal) const;
    std::shared_ptr<const eddic::Type> operator()(const ast::CharLiteral& literal) const;
    std::shared_ptr<const eddic::Type> operator()(const ast::Integer& literal) const;
    std::shared_ptr<const eddic::Type> operator()(const ast::IntegerSuffix& literal) const;
    std::shared_ptr<const eddic::Type> operator()(const ast::Float& literal) const;
    std::shared_ptr<const eddic::Type> operator()(const ast::VariableValue& variable) const;
    std::shared_ptr<const eddic::Type> operator()(const ast::Expression& value) const; 
    std::shared_ptr<const eddic::Type> operator()(const ast::FunctionCall& value) const; 
    std::shared_ptr<const eddic::Type> operator()(const ast::BuiltinOperator& value) const; 
    std::shared_ptr<const eddic::Type> operator()(const ast::True& value) const;
    std::shared_ptr<const eddic::Type> operator()(const ast::False& value) const;
    std::shared_ptr<const eddic::Type> operator()(const ast::Null& value) const;
    std::shared_ptr<const eddic::Type> operator()(const ast::Cast& cast) const;
    std::shared_ptr<const eddic::Type> operator()(const ast::PrefixOperation& value) const;
    std::shared_ptr<const eddic::Type> operator()(const ast::New& value) const;
    std::shared_ptr<const eddic::Type> operator()(const ast::NewArray& value) const;
    std::shared_ptr<const eddic::Type> operator()(const std::shared_ptr<Variable> value) const;
};

std::shared_ptr<const eddic::Type> operation_type(std::shared_ptr<const eddic::Type> left, std::shared_ptr<Context> context, const ast::Operation& operation);

} //end of ast

} //end of eddic

#endif
