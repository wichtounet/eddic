//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_TEMPLATE_FUNCTION_DECLARATION_H
#define AST_TEMPLATE_FUNCTION_DECLARATION_H

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/FunctionParameter.hpp"
#include "ast/Instruction.hpp"
#include "ast/Position.hpp"
#include "ast/VariableType.hpp"

namespace eddic {

class FunctionContext;

namespace ast {

/*!
 * \class ASTTemplateFunctionDeclaration
 * \brief The AST node for a template function declaration.  
 * Should only be used from the Deferred version (eddic::ast::TemplateFunctionDeclaration).
 */
struct ASTTemplateFunctionDeclaration { 
    std::shared_ptr<FunctionContext> context;
    
    std::string mangledName;
    std::string struct_name;

    Position position;
    std::vector<std::string> template_types;
    Type returnType;
    std::string functionName;
    std::vector<FunctionParameter> parameters;
    std::vector<Instruction> instructions;

    mutable long references = 0;
};

/*!
 * \typedef TemplateFunctionDeclaration
 * \brief The AST node for a template function declaration.
 */
typedef Deferred<ASTTemplateFunctionDeclaration> TemplateFunctionDeclaration;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::TemplateFunctionDeclaration, 
    (eddic::ast::Position, Content->position)
    (std::vector<std::string>, Content->template_types)
    (eddic::ast::Type, Content->returnType)
    (std::string, Content->functionName)
    (std::vector<eddic::ast::FunctionParameter>, Content->parameters)
    (std::vector<eddic::ast::Instruction>, Content->instructions)
)

#endif
