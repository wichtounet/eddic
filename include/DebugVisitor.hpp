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

    void operator()(ast::Program& program) const {
        std::cout << indent() << "Program" << std::endl; 

        ++level;

        visit_each(*this, program.Content->blocks);
    }

    void operator()(ast::FunctionDeclaration& declaration) const {
        std::cout << indent() << "Function " << declaration.Content->functionName << std::endl; 
   
        ++level;
        visit_each(*this, declaration.Content->instructions);    
        --level;
    }

    void operator()(ast::GlobalVariableDeclaration&) const {
        std::cout << indent() << "Global Variable" << std::endl; 
    }
    
    void operator()(ast::GlobalArrayDeclaration&) const {
        std::cout << indent() << "Global Array" << std::endl; 
    }

    void operator()(ast::For& for_) const {
        std::cout << indent() << "For" << std::endl; 
   
        ++level;
        visit_each(*this, for_.Content->instructions);    
        --level;
    }

    void operator()(ast::Foreach& for_) const {
        std::cout << indent() << "Foreach" << std::endl; 
   
        ++level;
        visit_each(*this, for_.Content->instructions);    
        --level;
    }

    void operator()(ast::While& while_) const {
        std::cout << indent() << "While" << std::endl; 
   
        ++level;
        visit_each(*this, while_.Content->instructions);    
        --level;
    }

    void operator()(ast::Swap&) const {
        std::cout << indent() << "Swap" << std::endl; 
    }

    void operator()(ast::If& if_) const {
        std::cout << indent() << "If" << std::endl; 
        ++level;
        visit_each(*this, if_.Content->instructions);    
        --level;
    }

    void operator()(ast::FunctionCall& call) const {
        std::cout << indent() << "FunctionCall " << call.Content->functionName << std::endl; 
        
        ++level;
        visit_each(*this, call.Content->values);
        --level;
    }

    void operator()(ast::VariableDeclaration& declaration) const {
        std::cout << indent() << "Variable declaration" << std::endl; 

        if(declaration.Content->value){
            ++level;
            visit(*this, *declaration.Content->value);
            --level;
        }
    }

    void operator()(ast::Assignment& assign) const {
        std::cout << indent() << "Variable assignment" << std::endl; 

        ++level;
        visit(*this, assign.Content->value);
        --level;
    }
    
    void operator()(ast::ArrayAssignment& assign) const {
        std::cout << indent() << "Array assignment" << std::endl; 

        ++level;
        visit(*this, assign.Content->value);
        --level;
    }

    void operator()(ast::Litteral&) const {
        std::cout << indent() << "Litteral" << std::endl; 
    }

    void operator()(ast::Integer& integer) const {
        std::cout << indent() << "Integer [" << integer.value << "]" << std::endl; 
    }

    void operator()(ast::VariableValue&) const {
        std::cout << indent() << "Variable" << std::endl; 
    }

    void operator()(ast::ArrayValue&) const {
        std::cout << indent() << "Array value" << std::endl; 
    }

    void operator()(ast::ComposedValue& value) const {
        std::cout << indent() << "Composed value [" << value.Content->operations.size() << "]" << std::endl; 
        ++level;
        visit(*this, value.Content->first);
        for(auto& operation : value.Content->operations){
            std::cout << indent() << operation.get<0>() << std::endl;
            visit(*this, operation.get<1>());
        }
        --level;
    }
};

} //end of eddic

#endif
