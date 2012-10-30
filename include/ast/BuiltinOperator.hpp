//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_BUILTIN_OPERATOR_H
#define AST_BUILTIN_OPERATOR_H

#include <vector>
#include <memory>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/Position.hpp"
#include "ast/Value.hpp"

namespace eddic {

namespace ast {

enum class BuiltinType : unsigned int {
    SIZE,
    LENGTH
};

std::ostream& operator<< (std::ostream& stream, BuiltinType type);

/*!
 * \class ASTFunctionCall
 * \brief The AST node for a function call. 
 * Should only be used from the Deferred version (eddic::ast::FunctionCall).
 */
struct ASTBuiltinOperator {
    Position position;
    BuiltinType type;
    std::vector<Value> values;

    mutable long references = 0;
};

/*!
 * \typedef FunctionCall
 * \brief The AST node for a function call.
 */
typedef Deferred<ASTBuiltinOperator> BuiltinOperator;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::BuiltinOperator, 
    (eddic::ast::Position, Content->position)
    (eddic::ast::BuiltinType, Content->type)
    (std::vector<eddic::ast::Value>, Content->values)
)

#endif
