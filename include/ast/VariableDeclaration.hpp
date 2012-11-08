//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_VARIABLE_DECLARATION_H
#define AST_VARIABLE_DECLARATION_H

#include <memory>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/Value.hpp"
#include "ast/Position.hpp"
#include "ast/VariableType.hpp"

namespace eddic {

class Context;

namespace ast {

/*!
 * \class ASTVariableDeclaration
 * \brief The AST node for a declaration of a local variable. 
 * Should only be used from the Deferred version (eddic::ast::VariableDeclaration).
 */
struct ASTVariableDeclaration {
    std::shared_ptr<Context> context;

    Position position;
    Type variableType;
    std::string variableName;
    boost::optional<Value> value;

    mutable long references = 0;
};

/*!
 * \typedef VariableDeclaration
 * \brief The AST node for a declaration of a local variable. 
 */
typedef Deferred<ASTVariableDeclaration> VariableDeclaration;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::VariableDeclaration, 
    (eddic::ast::Position, Content->position)
    (eddic::ast::Type, Content->variableType)
    (std::string, Content->variableName)
    (boost::optional<eddic::ast::Value>, Content->value)
)

#endif
