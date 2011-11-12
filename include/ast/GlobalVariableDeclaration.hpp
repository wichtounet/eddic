//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_GLOBAL_VARIABLE_DECLARATION_H
#define AST_GLOBAL_VARIABLE_DECLARATION_H

#include <memory>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Value.hpp"

namespace eddic {

class Context;

namespace ast {

struct ASTGlobalVariableDeclaration {
    std::shared_ptr<Context> context;
    
    std::string variableType;
    std::string variableName;
    Value value;
};

typedef Deferred<ASTGlobalVariableDeclaration> GlobalVariableDeclaration;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::GlobalVariableDeclaration, 
    (std::string, Content->variableType)
    (std::string, Content->variableName)
    (eddic::ast::Value, Content->value)
)

#endif
