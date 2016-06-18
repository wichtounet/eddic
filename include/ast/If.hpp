//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_IF_H
#define AST_IF_H

#include <vector>
#include <memory>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/ElseIf.hpp"
#include "ast/Else.hpp"
#include "ast/Value.hpp"

namespace eddic {

class Context;

namespace ast {

/*!
 * \class If
 * \brief The AST node for a if.
 */
struct If {
    std::shared_ptr<Context> context;

    Value condition;
    std::vector<Instruction> instructions;
    std::vector<ElseIf> elseIfs;
    boost::optional<Else> else_;

    mutable long references = 0;
};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::If,
    (eddic::ast::Value, condition)
    (std::vector<eddic::ast::Instruction>, instructions)
    (std::vector<eddic::ast::ElseIf>, elseIfs)
    (boost::optional<eddic::ast::Else>, else_)
)

#endif
