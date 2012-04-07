//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_STRUCT_H
#define AST_STRUCT_H

#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/MemberDeclaration.hpp"

namespace eddic {

namespace ast {

struct ASTStruct {
    std::vector<MemberDeclaration> declarations;
};

typedef Deferred<ASTStruct> Struct;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::Struct,
    (std::vector<eddic::ast::MemberDeclaration>, Content->declarations)
)

#endif
