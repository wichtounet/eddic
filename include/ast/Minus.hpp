//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_MINUS_H
#define AST_MINUS_H

#include <memory>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"

namespace eddic {

namespace ast {

struct ASTMinus {
    std::shared_ptr<Context> context;
    Value value;

    mutable long references;
    ASTMinus() : references(0) {}
};

typedef Deferred<ASTMinus> Minus;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::Minus, 
    (eddic::ast::Value, Content->value)
)

#endif
