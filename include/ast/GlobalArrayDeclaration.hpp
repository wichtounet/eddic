//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_GLOBAL_ARRAY_DECLARATION_H
#define AST_GLOBAL_ARRAY_DECLARATION_H

#include <memory>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/VariableType.hpp"

namespace eddic {

class Context;

namespace ast {

/*!
 * \class ASTGlobalArrayDeclaration
 * \brief The AST node for a global array declaration.   
 * Should only be used from the Deferred version (eddic::ast::GlobalarrayDeclaration).
 */
struct ASTGlobalArrayDeclaration {
    std::shared_ptr<Context> context;
    
    Position position;
    Type arrayType;
    std::string arrayName;
    int arraySize;

    mutable long references = 0;
};

/*!
 * \typedef GlobalArrayDeclaration
 * \brief The AST node for a global array declaration.
 */
typedef Deferred<ASTGlobalArrayDeclaration> GlobalArrayDeclaration;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::GlobalArrayDeclaration, 
    (eddic::ast::Position, Content->position)
    (eddic::ast::Type, Content->arrayType)
    (std::string, Content->arrayName)
    (int, Content->arraySize)
)

#endif
