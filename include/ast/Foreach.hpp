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

#include "ast/Deferred.hpp"

namespace eddic {

class Context;

namespace ast {

struct ASTForeach {
    std::shared_ptr<Context> context;
    std::string variableType;
    std::string variableName;
    int from;
    int to;
    std::vector<Instruction> instructions;
};

typedef Deferred<ASTForeach> Foreach;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::Foreach, 
    (std::string, Content->variableType)
    (std::string, Content->variableName)
    (int, Content->from)
    (int, Content->to)
    (std::vector<eddic::ast::Instruction>, Content->instructions)
)

#endif
