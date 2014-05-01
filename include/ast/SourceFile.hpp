//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_SOURCE_FILE_H
#define AST_SOURCE_FILE_H

#include <vector>

//Must be done before any other boost include
#include "variant.hpp"

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/home/support/attributes.hpp>
#include <boost/spirit/home/classic/iterator.hpp>

#include "ast/Position.hpp"
#include "ast/Deferred.hpp"
#include "ast/GlobalVariableDeclaration.hpp"
#include "ast/GlobalArrayDeclaration.hpp"
#include "ast/StandardImport.hpp"
#include "ast/Import.hpp"
#include "ast/Struct.hpp"
#include "ast/struct_definition.hpp"
#include "ast/FunctionDeclaration.hpp"
#include "ast/TemplateFunctionDeclaration.hpp"

#include "Context.hpp"

namespace eddic {

struct GlobalContext;

namespace ast {

/*!
 * \typedef SourceFileBlock
 * \brief A first level block in a source file.
 */
typedef boost::variant<
            FunctionDeclaration, 
            TemplateFunctionDeclaration, 
            GlobalVariableDeclaration,
            GlobalArrayDeclaration,
            StandardImport,
            Import,
            struct_definition,
            Struct
        > SourceFileBlock;

/*!
 * \class ASTSourceFile
 * \brief The AST root node for a program.
 * Should only be used from the Deferred version (eddic::ast::SourceFile).
 */
struct ASTSourceFile {
    std::shared_ptr<GlobalContext> context;

    Position position;
    std::vector<SourceFileBlock> blocks;

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
    (std::vector<eddic::ast::SourceFileBlock>, Content->blocks)
)

#endif
