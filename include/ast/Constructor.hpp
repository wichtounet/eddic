//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_CONSTRUCTOR_H
#define AST_CONSTRUCTOR_H

#include <string>
#include <memory>
#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/FunctionParameter.hpp"
#include "ast/Instruction.hpp"
#include "ast/Position.hpp"

namespace eddic {

namespace ast {

/*!
 * \class ASTConstructor
 * \brief The AST node for a constructor declaration.  
 * Should only be used from the Deferred version (eddic::ast::Constructor).
 */
struct ASTConstructor { 
    std::shared_ptr<FunctionContext> context;
    
    std::string mangledName;
    std::shared_ptr<const eddic::Type> struct_type = nullptr;

    Position position;
    std::vector<FunctionParameter> parameters;
    std::vector<Instruction> instructions;

    mutable long references = 0;
};

/*!
 * \typedef Constructor
 * \brief The AST node for a constructor declaration.
 */
typedef Deferred<ASTConstructor> Constructor; 

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::Constructor, 
    (eddic::ast::Position, Content->position)
    (std::vector<eddic::ast::FunctionParameter>, Content->parameters)
    (std::vector<eddic::ast::Instruction>, Content->instructions)
)

#endif
