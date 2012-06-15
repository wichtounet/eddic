//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_DEREFERENCE_VARIABLE_VALUE_H
#define AST_DEREFERENCE_VARIABLE_VALUE_H

#include <memory>
#include <vector>
#include <string>

#include "ast/Deferred.hpp"
#include "ast/Position.hpp"

namespace eddic {

class Context;
class Variable;

namespace ast {

/*!
 * \class ASTDereferenceVariableValue
 * \brief The AST node for a variable value.  
 * Should only be used from the Deferred version (eddic::ast::VariableValue).
 */
struct ASTDereferenceVariableValue {
    std::shared_ptr<Context> context;
    std::shared_ptr<Variable> var;

    Position position;
    char op;
    std::string variableName;
    std::vector<std::string> memberNames;

    mutable long references = 0;
};

/*!
 * \typedef DereferenceVariableValue
 * \brief The AST node for a variable value.
*/
typedef Deferred<ASTDereferenceVariableValue> DereferenceVariableValue;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::DereferenceVariableValue, 
    (eddic::ast::Position, Content->position)
    (char, Content->op)
    (std::string, Content->variableName)
    (std::vector<std::string>, Content->memberNames)
)

#endif
