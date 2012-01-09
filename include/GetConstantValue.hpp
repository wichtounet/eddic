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

typedef boost::variant<int, std::pair<std::string, int>> Val;

/*!
 * \class GetConstantValue
 * \brief AST Visitor to retrieve the constant value of an AST node. 
 * This visitor should only be used on a constant node.
 * \see eddic::IsConstantVisitor 
 */
struct GetConstantValue : public boost::static_visitor<Val> {
    Val operator()(const ast::Litteral& litteral) const;
    Val operator()(const ast::Integer& litteral) const;
    Val operator()(const ast::VariableValue& variable) const;

    template<typename T>
    Val operator()(const T&) const {
        assert(false); //Everything else is not constant
    }
};

} //end of eddic

#endif
