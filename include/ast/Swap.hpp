//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_SWAP_H
#define AST_SWAP_H

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Node.hpp"

namespace eddic {

struct ASTSwap : public virtual Node {
    std::string lhs;
    std::string rhs;
};

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ASTSwap, 
    (std::string, lhs)
    (std::string, rhs)
)

#endif
