//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_FOREACH_IN_H
#define AST_FOREACH_IN_H

#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"

namespace eddic {

class Context;
class Variable;

namespace ast {

/*!
 * \class ASTForeachIn
 * \brief The AST node for a foreach loop over an array. 
 * Should only be used from the Deferred version (eddic::ast::ForeachIn).
 */
struct ASTForeachIn {
    std::shared_ptr<Context> context;

    std::string variableType;
    std::string variableName;
    std::string arrayName;

    std::shared_ptr<Variable> var;
    std::shared_ptr<Variable> arrayVar;
    std::shared_ptr<Variable> iterVar;

    std::vector<Instruction> instructions;

    mutable long references;
    ASTForeachIn() : references(0) {}
};

/*!
 * \typedef ForeachIn
 * \brief The AST node for a foreach loop over an array.
 */
typedef Deferred<ASTForeachIn> ForeachIn;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::ForeachIn, 
    (std::string, Content->variableType)
    (std::string, Content->variableName)
    (std::string, Content->arrayName)
    (std::vector<eddic::ast::Instruction>, Content->instructions)
)

#endif
