//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_SOURCE_FILE_H
#define AST_SOURCE_FILE_H

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

/*!
 * \class ASTSourceFile
 * \brief The AST root node for a program.
 * Should only be used from the Deferred version (eddic::ast::Program).
 */
struct ASTSourceFile {
    std::shared_ptr<GlobalContext> context;

    std::vector<FirstLevelBlock> blocks;

    mutable long references;
    ASTSourceFile() : references(0) {}
};

/*!
 * \typedef Program
 * \brief The AST root node for a program.
 */
typedef Deferred<ASTSourceFile> SourceFile;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::SourceFile, 
    (std::vector<eddic::ast::FirstLevelBlock>, Content->blocks)
)

#endif
