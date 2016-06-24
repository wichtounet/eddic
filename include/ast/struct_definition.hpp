//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_STRUCT_DEFINITION_HPP
#define AST_STRUCT_DEFINITION_HPP

#include <vector>
#include <string>
#include <memory>

//Must be done before any other boost include
#include "variant.hpp"

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/optional.hpp>

#include "Type.hpp"

#include "ast/Position.hpp"
#include "ast/ArrayDeclaration.hpp"
#include "ast/MemberDeclaration.hpp"
#include "ast/Constructor.hpp"
#include "ast/Destructor.hpp"
#include "ast/TemplateFunctionDeclaration.hpp"

namespace eddic {

namespace ast {

/*!
 * \typedef StructBlock
 * \brief A block inside a structure.
 */
typedef x3::variant<
            MemberDeclaration,
            ArrayDeclaration,
            Constructor,
            Destructor,
            TemplateFunctionDeclaration
        > StructBlock;

struct struct_definition : x3::file_position_tagged {
    bool standard = false;
    std::string header = "";

    std::shared_ptr<const eddic::Type> struct_type = nullptr;

    Position position;
    std::vector<std::string> decl_template_types;
    std::string name;
    boost::optional<Type> parent_type;
    std::vector<StructBlock> blocks;

    std::vector<ast::Type> inst_template_types;

    mutable long references = 0;

    bool is_template_declaration(){
        return !decl_template_types.empty();
    }

    bool is_template_instantation(){
        return !inst_template_types.empty();
    }
};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::struct_definition,
    (std::vector<std::string>, decl_template_types)
    (std::string, name)
    (boost::optional<eddic::ast::Type>, parent_type)
    (std::vector<eddic::ast::StructBlock>, blocks)
)

#endif
