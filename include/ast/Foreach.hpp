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

/*!
 * \class ASTForeach
 * \brief The AST node for a foreach loop. 
 * Should only be used from the Deferred version (eddic::ast::Foreach).
 */
struct ASTForeach {
    std::shared_ptr<Context> context;

    ast::Position position;
    std::string variableType;
    std::string variableName;
    int from;
    int to;
    std::vector<Instruction> instructions;

    mutable long references;
    ASTForeach() : references(0) {}
};

/*!
 * \typedef Foreach
 * \brief The AST node for a foreach loop.
 */
typedef Deferred<ASTForeach> Foreach;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::Foreach, 
    (eddic::ast::Position, Content->position)
    (std::string, Content->variableType)
    (std::string, Content->variableName)
    (int, Content->from)
    (int, Content->to)
    (std::vector<eddic::ast::Instruction>, Content->instructions)
)

#endif
