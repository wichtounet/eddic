//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef IS_CONSTANT_VISITOR_H
#define IS_CONSTANT_VISITOR_H

#include <boost/variant/static_visitor.hpp>

#include "ast/values_def.hpp"

namespace eddic {

/*!
 * \struct IsConstantVisitor
 * \brief AST Visitor to test if a node is constant. 
 */
struct IsConstantVisitor : public boost::static_visitor<bool> {
    bool operator()(ast::Litteral& litteral) const;
    bool operator()(ast::Integer& litteral) const;
    bool operator()(ast::VariableValue& variable) const;
    bool operator()(ast::ArrayValue& variable) const;
    bool operator()(ast::ComposedValue& value) const; 
    bool operator()(ast::FunctionCall& value) const; 
};

} //end of eddic

#endif
