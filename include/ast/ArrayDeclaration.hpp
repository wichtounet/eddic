//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_ARRAY_DECLARATION_H
#define AST_ARRAY_DECLARATION_H

#include <memory>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/Position.hpp"

namespace eddic {

class Context;

namespace ast {

/*!
 * \class ASTArrayDeclaration
 * \brief The AST node for a declaration of a local array.  
 * Should only be used from the Deferred version (eddic::ast::ArrayDeclaration).
 */
struct ASTArrayDeclaration {
    std::shared_ptr<Context> context;

    Position position;
    std::string arrayType;
    std::string arrayName;
    int arraySize;

    mutable long references;
    ASTArrayDeclaration() : references(0) {}
};

/*!
 * \typedef ArrayDeclaration
 * \brief The AST node for a declaration of an array. 
 */
typedef Deferred<ASTArrayDeclaration> ArrayDeclaration;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::ArrayDeclaration, 
    (eddic::ast::Position, Content->position)
    (std::string, Content->arrayType)
    (std::string, Content->arrayName)
    (int, Content->arraySize)
)

#endif
