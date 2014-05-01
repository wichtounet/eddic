//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_TEMPLATE_STRUCT_H
#define AST_TEMPLATE_STRUCT_H

#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/optional.hpp>

#include "ast/Deferred.hpp"
#include "ast/Position.hpp"
#include "ast/Struct.hpp"

namespace eddic {

namespace ast {

struct ast_struct_definition {
    bool standard = false;
    std::string header = "";

    Position position;
    std::vector<std::string> template_types;
    std::string name;
    boost::optional<Type> parent_type;
    std::vector<StructBlock> blocks;

    mutable long references = 0;
};

typedef Deferred<ast_struct_definition> struct_definition;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::struct_definition,
    (eddic::ast::Position, Content->position)
    (std::vector<std::string>, Content->template_types)
    (std::string, Content->name)
    (boost::optional<eddic::ast::Type>, Content->parent_type)
    (std::vector<eddic::ast::StructBlock>, Content->blocks)
)

#endif
