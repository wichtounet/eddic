//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef IS_CONSTANT_VISITOR_H
#define IS_CONSTANT_VISITOR_H

#include "variant.hpp"

#include "ast/values_def.hpp"

namespace eddic {

namespace ast {

/*!
 * \struct IsConstantVisitor
 * \brief AST Visitor to test if a node is constant. 
 */
struct IsConstantVisitor : public boost::static_visitor<bool> {
    bool operator()(Litteral& litteral) const;
    bool operator()(Integer& litteral) const;
    bool operator()(IntegerSuffix& litteral) const;
    bool operator()(Float& litteral) const;
    bool operator()(Cast& cast) const;
    bool operator()(Plus& plus) const;
    bool operator()(Null& null) const;
    bool operator()(True& true_) const;
    bool operator()(False& false_) const;
    bool operator()(Minus& minus) const;
    bool operator()(VariableValue& variable) const;
    bool operator()(DereferenceValue& variable) const;
    bool operator()(ArrayValue& variable) const;
    bool operator()(Expression& value) const; 
    bool operator()(FunctionCall& value) const; 
    bool operator()(BuiltinOperator& value) const; 
    bool operator()(SuffixOperation& value) const; 
    bool operator()(PrefixOperation& value) const; 
    bool operator()(Assignment& assign) const; 
    bool operator()(Ternary& ternary) const; 
};

} //end of ast

} //end of eddic

#endif
