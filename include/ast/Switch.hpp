//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_SWITCH_H
#define AST_SWITCH_H

#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/Value.hpp"
#include "ast/SwitchCase.hpp"
#include "ast/DefaultCase.hpp"
#include "ast/Position.hpp"

namespace eddic {

namespace ast {

/*!
 * \class ASTSwitch
 * \brief The AST node for a switch case.
 * Should only be used from the Deferred version (eddic::ast::Switch).
 */
struct ASTSwitch {
    Position position;
    Value value;
    std::vector<SwitchCase> cases;
    boost::optional<DefaultCase> default_case;

    mutable long references = 0;
};

/*!
 * \typedef Switch
 * \brief The AST node for a switch. 
 */
typedef Deferred<ASTSwitch> Switch;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::Switch, 
    (eddic::ast::Position, Content->position)
    (eddic::ast::Value, Content->value)
    (std::vector<eddic::ast::SwitchCase>, Content->cases)
    (boost::optional<eddic::ast::DefaultCase>, Content->default_case)
)

#endif
