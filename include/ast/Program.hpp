//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_PROGRAM_H
#define AST_PROGRAM_H

#include <vector>

#include <boost/variant/variant.hpp>

#include <boost/fusion/include/adapt_struct.hpp>

#include <boost/spirit/home/support/attributes.hpp>

#include "ast/Deferred.hpp"
#include "ast/FunctionDeclaration.hpp"
#include "ast/GlobalVariableDeclaration.hpp"
#include "ast/GlobalArrayDeclaration.hpp"

#include "Context.hpp"

namespace eddic {

class Context;

namespace ast {

typedef boost::variant<
            FunctionDeclaration, 
            GlobalVariableDeclaration,
            GlobalArrayDeclaration
        > FirstLevelBlock;

//A source EDDI program
struct ASTProgram {
    std::shared_ptr<Context> context;
    std::vector<FirstLevelBlock> blocks;
};

typedef Deferred<ASTProgram> Program;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::Program, 
    (std::vector<eddic::ast::FirstLevelBlock>, Content->blocks)
)

#endif
