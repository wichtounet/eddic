//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_PLUS_H
#define AST_PLUS_H

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"

namespace eddic {

namespace ast {

struct ASTPlus {
    Value value;
    char op;

    mutable long references;
    ASTPlus() : references(0) {}
};

typedef Deferred<ASTPlus> Plus;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::Plus, 
    (char, Content->op)
    (eddic::ast::Value, Content->value)
)

#endif
