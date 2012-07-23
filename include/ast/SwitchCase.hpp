//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_SWITCH_CASE_H
#define AST_SWITCH_CASE_H

#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Value.hpp"
#include "ast/Position.hpp"

namespace eddic {

namespace ast {

/*!
 * \class SwitchCase 
 * \brief The AST node for a switch case.
 */
struct SwitchCase {
    Position position;
    Value value;
    std::vector<Instruction> instructions;
};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::SwitchCase, 
    (eddic::ast::Position, position)
    (eddic::ast::Value, value)
    (std::vector<eddic::ast::Instruction>, instructions)
)

#endif
