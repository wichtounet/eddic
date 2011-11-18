//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_ARRAY_ASSIGNMENT_H
#define AST_ARRAY_ASSIGNMENT_H

#include <memory>

#include <boost/intrusive_ptr.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/Value.hpp"

namespace eddic {

class Context;

namespace ast {

struct ASTArrayAssignment {
    std::shared_ptr<Context> context;

    std::string variableName;
    Value indexValue;
    Value value;

    mutable long references;
    ASTArrayAssignment() : references(0) {}
};

typedef Deferred<ASTArrayAssignment, boost::intrusive_ptr<ASTArrayAssignment>> ArrayAssignment;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::ArrayAssignment, 
    (std::string, Content->variableName)
    (eddic::ast::Value, Content->indexValue)
    (eddic::ast::Value, Content->value)
)

#endif
