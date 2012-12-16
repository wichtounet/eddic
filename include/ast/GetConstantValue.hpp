//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef GET_CONSTANT_VALUE_H
#define GET_CONSTANT_VALUE_H

#include <string>
#include <utility>

#include "assert.hpp"
#include "variant.hpp"

#include "ast/values_def.hpp"

namespace eddic {

typedef boost::variant<int, double, std::pair<std::string, int>> Val;

namespace ast {

/*!
 * \class GetConstantValue
 * \brief AST Visitor to retrieve the constant value of an AST node. 
 * This visitor should only be used on a constant node.
 * \see eddic::ast::IsConstantVisitor 
 */
struct GetConstantValue : public boost::static_visitor<Val> {
    Val operator()(const Literal& literal) const;
    Val operator()(const Integer& literal) const;
    Val operator()(const IntegerSuffix& literal) const;
    Val operator()(const Float& literal) const;
    Val operator()(const VariableValue& variable) const;
    Val operator()(const PrefixOperation& unary) const;

    template<typename T>
    Val operator()(const T&) const {
        eddic_unreachable("Is not constant");
    }
};

} //end of ast

} //end of eddic

#endif
