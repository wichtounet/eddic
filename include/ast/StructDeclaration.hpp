//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_STRUCT_DECLARATION_H
#define AST_STRUCT_DECLARATION_H

#include <memory>
#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/Value.hpp"
#include "ast/Position.hpp"
#include "ast/VariableType.hpp"

namespace eddic {

class Context;

namespace ast {

/*!
 * \class ASTStructDeclaration
 * \brief The AST node for a declaration of a local struct. 
 * Should only be used from the Deferred version (eddic::ast::StructDeclaration).
 */
struct ASTStructDeclaration {
    std::shared_ptr<Context> context;

    Position position;
    Type variableType;
    std::string variableName;
    std::vector<Value> values;

    mutable long references = 0;
};

/*!
 * \typedef StructDeclaration
 * \brief The AST node for a declaration of a local struct. 
 */
typedef Deferred<ASTStructDeclaration> StructDeclaration;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::StructDeclaration,
    (eddic::ast::Position, Content->position)
    (eddic::ast::Type, Content->variableType)
    (std::string, Content->variableName)
    (std::vector<eddic::ast::Value>, Content->values)
)

#endif
