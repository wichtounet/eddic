//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_FUNCTION_DECLARATION_H
#define AST_FUNCTION_DECLARATION_H

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/FunctionParameter.hpp"
#include "ast/Instruction.hpp"
#include "ast/Position.hpp"
#include "ast/VariableType.hpp"

namespace eddic {

class FunctionContext;

namespace ast {

/*!
 * \class ASTFunctionDeclaration
 * \brief The AST node for a function declaration.  
 * Should only be used from the Deferred version (eddic::ast::FunctionDeclaration).
 */
struct ASTFunctionDeclaration { 
    std::shared_ptr<FunctionContext> context;
    
    std::string mangledName;
    std::string struct_name;
    std::shared_ptr<const eddic::Type> struct_type = nullptr;

    bool instantiated = false;  /*!< True indicates that this function has been instantiated from a template */
    bool marked = false;        /*!< Indicates that the function has been treated by the front end */

    Position position;
    Type returnType;
    std::string functionName;
    std::vector<FunctionParameter> parameters;
    std::vector<Instruction> instructions;

    mutable long references = 0;
};

/*!
 * \typedef FunctionDeclaration
 * \brief The AST node for a function declaration.
 */
typedef Deferred<ASTFunctionDeclaration> FunctionDeclaration; 

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::FunctionDeclaration, 
    (eddic::ast::Position, Content->position)
    (eddic::ast::Type, Content->returnType)
    (std::string, Content->functionName)
    (std::vector<eddic::ast::FunctionParameter>, Content->parameters)
    (std::vector<eddic::ast::Instruction>, Content->instructions)
)

#endif
