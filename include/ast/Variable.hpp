//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_VARIABLE_H
#define AST_VARIABLE_H

#include <memory>

namespace eddic {

class Context;
class Variable;

struct ASTVariable {
    std::shared_ptr<Context> context;

    std::string variableName;
    std::shared_ptr<Variable> var;
};

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ASTVariable, 
    (std::string, variableName)
)

#endif
