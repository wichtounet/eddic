//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_SOURCE_FILE_H
#define AST_SOURCE_FILE_H

#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/home/support/attributes.hpp>
#include <boost/spirit/home/classic/iterator.hpp>

#include "variant.hpp"

#include "ast/Position.hpp"
#include "ast/Deferred.hpp"
#include "ast/FunctionDeclaration.hpp"
#include "ast/GlobalVariableDeclaration.hpp"
#include "ast/GlobalArrayDeclaration.hpp"
#include "ast/StandardImport.hpp"
#include "ast/Import.hpp"
#include "ast/Struct.hpp"

#include "Context.hpp"

namespace eddic {

class GlobalContext;

namespace ast {

/*!
 * \typedef SourceFile
 * \brief A first level block in a source file.
 */
typedef boost::variant<
            FunctionDeclaration, 
            GlobalVariableDeclaration,
            GlobalArrayDeclaration,
            StandardImport,
            Import,
            Struct
        > FirstLevelBlock;

/*!
 * \class ASTSourceFile
 * \brief The AST root node for a program.
 * Should only be used from the Deferred version (eddic::ast::SourceFile).
 */
struct ASTSourceFile {
    std::shared_ptr<GlobalContext> context;

    Position position;
    std::vector<FirstLevelBlock> blocks;

    mutable long references = 0;
};

/*!
 * \typedef SourceFile
 * \brief The AST root node for a program.
 */
typedef Deferred<ASTSourceFile> SourceFile;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::SourceFile, 
    (eddic::ast::Position, Content->position)
    (std::vector<eddic::ast::FirstLevelBlock>, Content->blocks)
)

#endif
