//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_FUNCTION_DECLARATION_H
#define AST_FUNCTION_DECLARATION_H

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Node.hpp"
#include "ast/FunctionParameter.hpp"
#include "ast/Instruction.hpp"

namespace eddic {

struct ASTFunctionDeclaration : public Node { 
    std::string returnType;
    std::string functionName;
    std::vector<FunctionParameter> parameters;
    std::vector<ASTInstruction> instructions;
};

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ASTFunctionDeclaration, 
    (std::string, returnType)
    (std::string, functionName)
    (std::vector<eddic::FunctionParameter>, parameters)
    (std::vector<eddic::ASTInstruction>, instructions)
)

#endif
