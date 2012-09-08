//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef GET_TYPE_VISITOR_H
#define GET_TYPE_VISITOR_H

#include <memory>

#include "variant.hpp"

#include "ast/values_def.hpp"

namespace eddic {

class Variable;

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
    std::shared_ptr<const eddic::Type> operator()(const ast::DereferenceValue& variable) const;
    std::shared_ptr<const eddic::Type> operator()(const ast::MemberValue& value) const;
    std::shared_ptr<const eddic::Type> operator()(const ast::ArrayValue& variable) const;
    std::shared_ptr<const eddic::Type> operator()(const ast::Expression& value) const; 
    std::shared_ptr<const eddic::Type> operator()(const ast::FunctionCall& value) const; 
    std::shared_ptr<const eddic::Type> operator()(const ast::MemberFunctionCall& value) const; 
    std::shared_ptr<const eddic::Type> operator()(const ast::BuiltinOperator& value) const; 
    std::shared_ptr<const eddic::Type> operator()(const ast::Unary& value) const;
    std::shared_ptr<const eddic::Type> operator()(const ast::True& value) const;
    std::shared_ptr<const eddic::Type> operator()(const ast::False& value) const;
    std::shared_ptr<const eddic::Type> operator()(const ast::Null& value) const;
    std::shared_ptr<const eddic::Type> operator()(const ast::Cast& cast) const;
    std::shared_ptr<const eddic::Type> operator()(const ast::SuffixOperation& value) const;
    std::shared_ptr<const eddic::Type> operator()(const ast::PrefixOperation& value) const;
    std::shared_ptr<const eddic::Type> operator()(const ast::New& value) const;
    std::shared_ptr<const eddic::Type> operator()(const ast::NewArray& value) const;
    std::shared_ptr<const eddic::Type> operator()(const std::shared_ptr<Variable> value) const;
};

} //end of ast

} //end of eddic

#endif
