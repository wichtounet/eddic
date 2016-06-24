//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_FUNCTION_CALL_H
#define AST_FUNCTION_CALL_H

#include <memory>
#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>

#include "Type.hpp"
#include "ast/Position.hpp"
#include "ast/Value.hpp"
#include "ast/VariableType.hpp"

namespace eddic {

class Context;

namespace ast {

/*!
 * \class ASTFunctionCall
 * \brief The AST node for a function call.
 */
struct FunctionCall : x3::file_position_tagged {
    std::shared_ptr<Context> context;
    std::string mangled_name;
    std::shared_ptr<const eddic::Type> left_type;

    Position position;
    std::string function_name;
    std::vector<ast::Type> template_types;
    std::vector<Value> values;

};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::FunctionCall,
    (std::string, function_name)
    (std::vector<eddic::ast::Type>, template_types)
    (std::vector<eddic::ast::Value>, values)
)

#endif
