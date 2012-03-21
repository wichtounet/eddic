//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef GET_TYPE_VISITOR_H
#define GET_TYPE_VISITOR_H

#include <boost/variant/static_visitor.hpp>

#include "Types.hpp"

#include "ast/values_def.hpp"

namespace eddic {

namespace ast {

/*!
 * \class GetTypeVisitor
 * \brief Static Visitor to get the Type of a Value from the AST. 
 */
struct GetTypeVisitor : public boost::static_visitor<eddic::Type> {
    eddic::Type operator()(const ast::Litteral& litteral) const;
    eddic::Type operator()(const ast::Integer& litteral) const;
    eddic::Type operator()(const ast::IntegerSuffix& litteral) const;
    eddic::Type operator()(const ast::Float& litteral) const;
    eddic::Type operator()(const ast::VariableValue& variable) const;
    eddic::Type operator()(const ast::ArrayValue& variable) const;
    eddic::Type operator()(const ast::Expression& value) const; 
    eddic::Type operator()(const ast::FunctionCall& value) const; 
    eddic::Type operator()(const ast::BuiltinOperator& value) const; 
    eddic::Type operator()(const ast::Assignment& assign) const; 
    eddic::Type operator()(const ast::Plus& value) const;
    eddic::Type operator()(const ast::Minus& value) const;
    eddic::Type operator()(const ast::True& value) const;
    eddic::Type operator()(const ast::False& value) const;
    eddic::Type operator()(const ast::Cast& cast) const;
    eddic::Type operator()(const ast::SuffixOperation& value) const;
    eddic::Type operator()(const ast::PrefixOperation& value) const;
};

} //end of ast

} //end of eddic

#endif
