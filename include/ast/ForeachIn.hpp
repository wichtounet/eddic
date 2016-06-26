//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_FOREACH_IN_H
#define AST_FOREACH_IN_H

#include <vector>
#include <memory>

#include <boost/fusion/include/adapt_struct.hpp>

namespace eddic {

class Context;
class Variable;

namespace ast {

/*!
 * \class ASTForeachIn
 * \brief The AST node for a foreach loop over an array.
 */
struct ForeachIn : x3::file_position_tagged {
    std::shared_ptr<Context> context;

    ast::Type variableType;
    std::string variableName;
    std::string arrayName;

    std::shared_ptr<Variable> var;
    std::shared_ptr<Variable> arrayVar;
    std::shared_ptr<Variable> iterVar;

    std::vector<Instruction> instructions;

};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::ForeachIn,
    (eddic::ast::Type, variableType)
    (std::string, variableName)
    (std::string, arrayName)
    (std::vector<eddic::ast::Instruction>, instructions)
)

#endif
