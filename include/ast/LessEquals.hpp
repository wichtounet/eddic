//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_LESS_EQUALS_H
#define AST_LESS_EQUALS_H

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/Value.hpp"

namespace eddic {

struct LessEquals {
    ASTValue lhs;
    ASTValue rhs;
};

typedef Deferred<LessEquals> ASTLessEquals;

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ASTLessEquals, 
    (eddic::ASTValue, Content->lhs)
    (eddic::ASTValue, Content->rhs)
)

#endif
