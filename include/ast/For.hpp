//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_FOR_H
#define AST_FOR_H

#include <vector>
#include <memory>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Value.hpp"

namespace eddic {

class Context;

namespace ast {

/*!
 * \class For
 * \brief The AST node for a for loop.
 */
struct For {
    std::shared_ptr<Context> context;

    boost::optional<Instruction> start;
    boost::optional<Value> condition;
    boost::optional<Instruction> repeat;
    std::vector<Instruction> instructions;

    mutable long references = 0;
};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::For,
    (boost::optional<eddic::ast::Instruction>, start)
    (boost::optional<eddic::ast::Value>, condition)
    (boost::optional<eddic::ast::Instruction>, repeat)
    (std::vector<eddic::ast::Instruction>, instructions)
)

#endif
