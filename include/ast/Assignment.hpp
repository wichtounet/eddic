//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_ASSIGNMENT_H
#define AST_ASSIGNMENT_H

#include <memory>

#include <boost/intrusive_ptr.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/Value.hpp"

namespace eddic {

class Context;

struct Assignment {
    std::shared_ptr<Context> context;

    std::string variableName;
    ASTValue value;

    long RefCount;
    Assignment() : RefCount(0) {}
};

typedef Deferred<Assignment, boost::intrusive_ptr<Assignment>> ASTAssignment;

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ASTAssignment, 
    (std::string, Content->variableName)
    (eddic::ASTValue, Content->value)
)

#endif
