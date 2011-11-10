//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_DECLARATION_H
#define AST_DECLARATION_H

#include <memory>

#include <boost/intrusive_ptr.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/Value.hpp"

namespace eddic {

class Context;

struct Declaration {
    std::shared_ptr<Context> context;

    std::string variableType;
    std::string variableName;
    ASTValue value;

    mutable long references;
    Declaration() : references(0) {}
};

typedef Deferred<Declaration, boost::intrusive_ptr<Declaration>> ASTDeclaration;

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ASTDeclaration, 
    (std::string, Content->variableType)
    (std::string, Content->variableName)
    (eddic::ASTValue, Content->value)
)

#endif
