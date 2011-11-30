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
#include "ast/StandardImport.hpp"
#include "ast/Import.hpp"

#include "Context.hpp"

namespace eddic {

class GlobalContext;

namespace ast {

typedef boost::variant<
            FunctionDeclaration, 
            GlobalVariableDeclaration,
            GlobalArrayDeclaration,
            StandardImport,
            Import
        > FirstLevelBlock;

//A source EDDI program
struct ASTProgram {
    std::shared_ptr<GlobalContext> context;

    std::vector<FirstLevelBlock> blocks;

    mutable long references;
    ASTProgram() : references(0) {}
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
