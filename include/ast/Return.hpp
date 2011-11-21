//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_RETURN_H
#define AST_RETURN_H

#include <memory>

#include <boost/intrusive_ptr.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/Value.hpp"

namespace eddic {

class Context;

namespace ast {

struct ASTReturn {
    Value value;

    mutable long references;
    ASTReturn() : references(0) {}
};

typedef Deferred<ASTReturn, boost::intrusive_ptr<ASTReturn>> Return;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::Assignment, 
    (eddic::ast::Value, Content->value)
)

#endif
