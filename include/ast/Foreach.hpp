//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_FOREACH_H
#define AST_FOREACH_H

#include <vector>
#include <memory>

#include <boost/fusion/include/adapt_struct.hpp>

namespace eddic {

class Context;

namespace ast {

/*!
 * \class Foreach
 * \brief The AST node for a foreach loop.
 */
struct Foreach : x3::position_tagged {
    std::shared_ptr<Context> context;

    ast::Position position;
    ast::Type variableType;
    std::string variableName;
    int from;
    int to;
    std::vector<Instruction> instructions;

    mutable long references = 0;
};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::Foreach,
    (eddic::ast::Type, variableType)
    (std::string, variableName)
    (int, from)
    (int, to)
    (std::vector<eddic::ast::Instruction>, instructions)
)

#endif
