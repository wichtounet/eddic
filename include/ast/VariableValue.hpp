//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_VARIABLE_VALUE_H
#define AST_VARIABLE_VALUE_H

#include <memory>

#include "ast/Deferred.hpp"
#include "ast/Position.hpp"

namespace eddic {

class Context;
class Variable;

namespace ast {

/*!
 * \class ASTVariableValue
 * \brief The AST node for a variable value.  
 * Should only be used from the Deferred version (eddic::ast::VariableValue).
 */
struct ASTVariableValue {
    std::shared_ptr<Context> context;

    Position position;
    std::string variableName;
    std::shared_ptr<Variable> var;

    mutable long references;
    ASTVariableValue() : references(0) {}
};

/*!
 * \typedef VariableValue
 * \brief The AST node for a variable value.
*/
typedef Deferred<ASTVariableValue> VariableValue;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::VariableValue, 
    (eddic::ast::Position, Content->position)
    (std::string, Content->variableName)
)

#endif
