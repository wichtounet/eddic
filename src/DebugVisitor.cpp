//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "DebugVisitor.hpp"

#include "VisitorUtils.hpp"

#include "ast/SourceFile.hpp"

using namespace eddic;
    
DebugVisitor::DebugVisitor() : level(0) {}

std::string DebugVisitor::indent() const {
    std::string acc = "";
    for(int i = 0; i < level; ++i){
        acc += "\t";
    }
    return acc;
}

void DebugVisitor::operator()(ast::SourceFile& program) const {
    std::cout << indent() << "SourceFile" << std::endl; 

    ++level;

    visit_each(*this, program.Content->blocks);
}

void DebugVisitor::operator()(ast::Import& import) const {
    std::cout << indent() << "include \"" << import.file << "\"" << std::endl;
}

void DebugVisitor::operator()(ast::StandardImport& import) const {
    std::cout << indent() << "include <" << import.header << ">" << std::endl;
}

void DebugVisitor::operator()(ast::FunctionDeclaration& declaration) const {
    std::cout << indent() << "Function " << declaration.Content->functionName << std::endl; 

    ++level;
    visit_each(*this, declaration.Content->instructions);    
    --level;
}

void DebugVisitor::operator()(ast::GlobalVariableDeclaration&) const {
    std::cout << indent() << "Global Variable" << std::endl; 
}

void DebugVisitor::operator()(ast::GlobalArrayDeclaration&) const {
    std::cout << indent() << "Global Array" << std::endl; 
}

void DebugVisitor::operator()(ast::For& for_) const {
    std::cout << indent() << "For" << std::endl; 

    ++level;
    visit_each(*this, for_.Content->instructions);    
    --level;
}

void DebugVisitor::operator()(ast::Foreach& for_) const {
    std::cout << indent() << "Foreach" << std::endl; 

    ++level;
    visit_each(*this, for_.Content->instructions);    
    --level;
}

void DebugVisitor::operator()(ast::ForeachIn& for_) const {
    std::cout << indent() << "Foreach in " << std::endl; 

    ++level;
    visit_each(*this, for_.Content->instructions);    
    --level;
}

void DebugVisitor::operator()(ast::While& while_) const {
    std::cout << indent() << "While" << std::endl; 

    ++level;
    visit_each(*this, while_.Content->instructions);    
    --level;
}

void DebugVisitor::operator()(ast::Swap&) const {
    std::cout << indent() << "Swap" << std::endl; 
}

void DebugVisitor::operator()(ast::If& if_) const {
    std::cout << indent() << "If" << std::endl; 
    std::cout << indent() << "Condition:" << std::endl;
    ++level;
    visit(*this, if_.Content->condition);    
    --level;

    std::cout << indent() << "Body:" << std::endl;
    ++level;
    visit_each(*this, if_.Content->instructions);    
    --level;
}

void DebugVisitor::operator()(ast::FunctionCall& call) const {
    std::cout << indent() << "FunctionCall " << call.Content->functionName << std::endl; 

    ++level;
    visit_each(*this, call.Content->values);
    --level;
}

void DebugVisitor::operator()(ast::Builtin& builtin) const {
    std::cout << indent() << "Builtin Operator " << (int) builtin.Content->type << std::endl; 

    ++level;
    visit_each(*this, builtin.Content->values);
    --level;
}

void DebugVisitor::operator()(ast::VariableDeclaration& declaration) const {
    std::cout << indent() << "Variable declaration" << std::endl; 

    if(declaration.Content->value){
        ++level;
        visit(*this, *declaration.Content->value);
        --level;
    }
}

void DebugVisitor::operator()(ast::ArrayDeclaration&) const {
    std::cout << indent() << "Array declaration" << std::endl; 
}

void DebugVisitor::operator()(ast::SuffixOperation& op) const {
    std::cout << indent() << op.Content->variableName << "(suffix)" << (int)op.Content->op << std::endl; 
}

void DebugVisitor::operator()(ast::PrefixOperation& op) const {
    std::cout << indent() << (int)op.Content->op << "(prefix)" << op.Content->variableName << std::endl; 
}

void DebugVisitor::operator()(ast::Assignment& assign) const {
    std::cout << indent() << "Variable assignment" << std::endl; 

    ++level;
    visit(*this, assign.Content->value);
    --level;
}

void DebugVisitor::operator()(ast::CompoundAssignment& assign) const {
    std::cout << indent() << "Compound variable assignment [operator = " << (int) assign.Content->op << " ]" << std::endl; 

    ++level;
    visit(*this, assign.Content->value);
    --level;
}

void DebugVisitor::operator()(ast::Return& return_) const {
    std::cout << indent() << "Function return" << std::endl; 

    ++level;
    visit(*this, return_.Content->value);
    --level;
}

void DebugVisitor::operator()(ast::ArrayAssignment& assign) const {
    std::cout << indent() << "Array assignment" << std::endl; 

    ++level;
    visit(*this, assign.Content->value);
    --level;
}

void DebugVisitor::operator()(ast::Litteral& litteral) const {
    std::cout << indent() << "Litteral [" << litteral.value << "]" << std::endl; 
}

void DebugVisitor::operator()(ast::Integer& integer) const {
    std::cout << indent() << "Integer [" << integer.value << "]" << std::endl; 
}

void DebugVisitor::operator()(ast::True&) const {
    std::cout << indent() << "true" << std::endl; 
}

void DebugVisitor::operator()(ast::False&) const {
    std::cout << indent() << "false" << std::endl; 
}

void DebugVisitor::operator()(ast::VariableValue&) const {
    std::cout << indent() << "Variable" << std::endl; 
}

void DebugVisitor::operator()(ast::ArrayValue&) const {
    std::cout << indent() << "Array value" << std::endl; 
}

void DebugVisitor::operator()(ast::ComposedValue& value) const {
    std::cout << indent() << "Composed value [" << value.Content->operations.size() << "]" << std::endl; 
    ++level;
    visit(*this, value.Content->first);
    for(auto& operation : value.Content->operations){
        std::cout << indent() << (int) operation.get<0>() << std::endl;
        visit(*this, operation.get<1>());
    }
    --level;
}

void DebugVisitor::operator()(ast::Minus& value) const {
    std::cout << indent() << "Unary +" << std::endl; 
    ++level;
    visit(*this, value.Content->value);
    --level;
}

void DebugVisitor::operator()(ast::Plus& value) const {
    std::cout << indent() << "Unary -" << std::endl; 
    ++level;
    visit(*this, value.Content->value);
    --level;
}
