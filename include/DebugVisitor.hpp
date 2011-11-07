//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_DEBUG_VISITOR_H
#define AST_DEBUG_VISITOR_H

#include <iostream>

#include <boost/variant/static_visitor.hpp>

#include "ast/Program.hpp"

#include "VisitorUtils.hpp"

namespace eddic {

struct DebugVisitor : public boost::static_visitor<> {
    mutable int level;

    DebugVisitor() : level(0) {}

    std::string indent() const {
        std::string acc = "";
        for(int i = 0; i < level; ++i){
            acc += "\t";
        }
        return acc;
    }

    void operator()(ASTProgram& program) const {
        std::cout << indent() << "Program" << std::endl; 

        ++level;

        visit_each(*this, program.blocks);
    }

    void operator()(ASTFunctionDeclaration& declaration) const {
        std::cout << indent() << "Function " << declaration.functionName << std::endl; 
   
        ++level;
        visit_each(*this, declaration.instructions);    
        --level;
    }

    void operator()(GlobalVariableDeclaration& declaration) const {
        std::cout << indent() << "GlobalVariable" << std::endl; 
    }

    void operator()(ASTFor& for_) const {
        std::cout << indent() << "For" << std::endl; 
   
        ++level;
        visit_each(*this, for_.instructions);    
        --level;
    }

    void operator()(ASTForeach& for_) const {
        std::cout << indent() << "Foreach" << std::endl; 
   
        ++level;
        visit_each(*this, for_.instructions);    
        --level;
    }

    void operator()(ASTWhile& while_) const {
        std::cout << indent() << "While" << std::endl; 
   
        ++level;
        visit_each(*this, while_.instructions);    
        --level;
    }

    void operator()(ASTSwap& swap) const {
        std::cout << indent() << "Swap" << std::endl; 
    }

    void operator()(ASTIf& if_) const {
        std::cout << indent() << "If" << std::endl; 
        ++level;
        visit_each(*this, if_.instructions);    
        --level;
    }

    void operator()(ASTFunctionCall& call) const {
        std::cout << indent() << "FunctionCall " << call.functionName << std::endl; 
        
        ++level;
        visit_each(*this, call.values);
        --level;
    }

    void operator()(ASTDeclaration& declaration) const {
        std::cout << indent() << "Variable declaration" << std::endl; 

        ++level;
        visit(*this, declaration.value);
        --level;
    }

    void operator()(ASTAssignment& assign) const {
        std::cout << indent() << "Variable assignment" << std::endl; 

        ++level;
        visit(*this, assign.value);
        --level;
    }

    void operator()(ASTLitteral& litteral) const {
        std::cout << indent() << "Litteral" << std::endl; 
    }

    void operator()(ASTInteger& integer) const {
        std::cout << indent() << "Integer" << std::endl; 
    }

    void operator()(ASTVariable& variable) const {
        std::cout << indent() << "Variable" << std::endl; 
    }

    void operator()(ASTComposedValue& value) const {
        std::cout << indent() << "Composed value [" << value.operations.size() << "]" << std::endl; 
        ++level;
        visit(*this, value.first);
        for(auto& operation : value.operations){
            std::cout << indent() << operation.get<0>() << std::endl;
            visit(*this, operation.get<1>());
        }
        --level;
    }
};

} //end of eddic

#endif
