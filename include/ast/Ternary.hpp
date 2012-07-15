//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_TERNARY_H
#define AST_TERNARY_H

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/Position.hpp"

namespace eddic {

namespace ast {

/*!
 * \class ASTTernary
 * \brief The AST node for a ternary operator.    
 * Should only be used from the Deferred version (eddic::ast::Ternary).
 */
struct ASTTernary {
    Position position;
    Value condition;
    Value true_value;
    Value false_value;

    mutable long references = 0;
};

/*!
 * \typedef If
 * \brief The AST node for a if. 
 */
typedef Deferred<ASTTernary> Ternary;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::Ternary, 
    (eddic::ast::Position, Content->position)
    (eddic::ast::Value, Content->condition)
    (eddic::ast::Value, Content->true_value)
    (eddic::ast::Value, Content->false_value)
)

#endif
