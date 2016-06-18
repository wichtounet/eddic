//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
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
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>

#include "ast/Position.hpp"
#include "ast/GlobalVariableDeclaration.hpp"
#include "ast/GlobalArrayDeclaration.hpp"
#include "ast/StandardImport.hpp"
#include "ast/Import.hpp"
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
typedef x3::variant<
            FunctionDeclaration,
            TemplateFunctionDeclaration,
            GlobalVariableDeclaration,
            GlobalArrayDeclaration,
            StandardImport,
            Import,
            struct_definition
        > SourceFileBlock;

/*!
 * \class ASTSourceFile
 * \brief The AST root node for a program.
 */
struct SourceFile {
    std::shared_ptr<GlobalContext> context;

    Position position;
    std::vector<SourceFileBlock> blocks;

    mutable long references = 0;
};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::SourceFile,
    (eddic::ast::Position, position)
    (std::vector<eddic::ast::SourceFileBlock>, blocks)
)

#endif
