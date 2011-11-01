//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_FOREACH_H
#define AST_FOREACH_H

#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Node.hpp"

namespace eddic {

struct ASTForeach : public virtual Node {
    std::string variableType;
    std::string variableName;
    int from;
    int to;
    std::vector<ASTInstruction> instructions;
};

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ASTForeach, 
    (std::string, variableType)
    (std::string, variableName)
    (int, from)
    (int, to)
    (std::vector<eddic::ASTInstruction>, instructions)
)

#endif
