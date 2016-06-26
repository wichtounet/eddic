//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_SWITCH_H
#define AST_SWITCH_H

#include <vector>
#include <memory>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Value.hpp"
#include "ast/SwitchCase.hpp"
#include "ast/DefaultCase.hpp"

namespace eddic {

class Context;

namespace ast {

/*!
 * \class ASTSwitch
 * \brief The AST node for a switch case.
 */
struct Switch : x3::file_position_tagged {
    std::shared_ptr<Context> context;

    Value value;
    std::vector<SwitchCase> cases;
    boost::optional<DefaultCase> default_case;

};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::Switch,
    (eddic::ast::Value, value)
    (std::vector<eddic::ast::SwitchCase>, cases)
    (boost::optional<eddic::ast::DefaultCase>, default_case)
)

#endif
