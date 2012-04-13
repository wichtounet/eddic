//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_STRUCT_VALUE_H
#define AST_STRUCT_VALUE_H

#include <memory>
#include <string>

#include <boost/optional.hpp>

#include "Type.hpp"

#include "ast/Deferred.hpp"
#include "ast/Position.hpp"

namespace eddic {

class Context;
class Variable;

namespace ast {

/*!
 * \class ASTStructValue
 * \brief The AST node for a value of an array. 
 * Should only be used from the Deferred version (eddic::ast::StructValue).
 */
struct ASTStructValue {
    std::shared_ptr<Context> context;

    Position position;
    std::string variableName;
    std::string memberName;

    std::shared_ptr<Variable> variable;
    boost::optional<eddic::Type> type;

    mutable long references = 0;
};

/*!
 * \typedef StructValue
 * \brief The AST node for a value of an array.
 */
typedef Deferred<ASTStructValue> StructValue;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::StructValue, 
    (eddic::ast::Position, Content->position)
    (std::string, Content->variableName)
    (std::string, Content->memberName)
)

#endif
