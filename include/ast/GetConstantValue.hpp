//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef GET_CONSTANT_VALUE_H
#define GET_CONSTANT_VALUE_H

#include <string>
#include <utility>

#include <boost/variant/get.hpp>
#include <boost/variant/static_visitor.hpp>

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
    Val operator()(const Litteral& litteral) const;
    Val operator()(const Integer& litteral) const;
    Val operator()(const Float& litteral) const;
    Val operator()(const VariableValue& variable) const;
    Val operator()(const Minus& minus) const;

    template<typename T>
    Val operator()(const T&) const {
        assert(false && "Value not constant");
    }
};

} //end of ast

} //end of eddic

#endif
