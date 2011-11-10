//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_SWAP_H
#define AST_SWAP_H

#include <memory>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"

namespace eddic {

class Context;
class Variable;

struct Swap {
    std::shared_ptr<Context> context;
    std::string lhs;
    std::string rhs;
    std::shared_ptr<Variable> lhs_var;
    std::shared_ptr<Variable> rhs_var;
};

typedef Deferred<Swap> ASTSwap;

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ASTSwap, 
    (std::string, Content->lhs)
    (std::string, Content->rhs)
)

#endif
