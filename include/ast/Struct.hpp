//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_STRUCT_H
#define AST_STRUCT_H

#include <vector>
#include <string>
#include <memory>

//Must be done before any other boost include
#include "variant.hpp"

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/optional.hpp>

#include "Type.hpp"

#include "ast/Deferred.hpp"
#include "ast/Position.hpp"
#include "ast/ArrayDeclaration.hpp"
#include "ast/MemberDeclaration.hpp"
#include "ast/Constructor.hpp"
#include "ast/Destructor.hpp"
#include "ast/FunctionDeclaration.hpp"
#include "ast/TemplateFunctionDeclaration.hpp"

namespace eddic {

namespace ast {

/*!
 * \typedef StructBlock
 * \brief A block inside a structure. 
 */
typedef boost::variant<
            MemberDeclaration,
            ArrayDeclaration,
            Constructor, 
            Destructor,
            FunctionDeclaration,
            TemplateFunctionDeclaration
        > StructBlock;

/*!
 * \class ASTStruct
 * \brief The AST node for a structure declaration.  
 * Should only be used from the Deferred version (eddic::ast::Struct).
 */
struct ASTStruct {
    std::vector<ast::Type> template_types;  /*!< Indicates with which types this class template has been instantiated */
    std::shared_ptr<const eddic::Type> struct_type;
    bool standard = false;
    std::string header = "";

    Position position;
    std::string name;
    boost::optional<Type> parent_type;
    std::vector<StructBlock> blocks;

    mutable long references = 0;
};

/*!
 * \typedef Struct
 * \brief The AST node for a structure declaration.
 */
typedef Deferred<ASTStruct> Struct;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::Struct,
    (eddic::ast::Position, Content->position)
    (std::string, Content->name)
    (boost::optional<eddic::ast::Type>, Content->parent_type)
    (std::vector<eddic::ast::StructBlock>, Content->blocks)
)

#endif
