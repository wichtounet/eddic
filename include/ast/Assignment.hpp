//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_ASSIGNMENT_H
#define AST_ASSIGNMENT_H

#include <memory>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/Value.hpp"

namespace eddic {

class Context;

namespace ast {

struct ASTAssignment {
    std::shared_ptr<Context> context;

    std::string variableName;
    Value value;

    mutable long references;
    ASTAssignment() : references(0) {}
};

typedef Deferred<ASTAssignment> Assignment;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::Assignment, 
    (std::string, Content->variableName)
    (eddic::ast::Value, Content->value)
)

#endif
