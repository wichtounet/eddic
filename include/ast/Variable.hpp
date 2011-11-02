//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_VARIABLE_H
#define AST_VARIABLE_H

#include "ast/Node.hpp"

namespace eddic {

struct ASTVariable : public Node {
    std::string variableName;
};

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ASTVariable, 
    (std::string, variableName)
)

#endif
