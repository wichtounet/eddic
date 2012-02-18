//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "ast/DebugVisitor.hpp"
#include "ast/SourceFile.hpp"

#include "VisitorUtils.hpp"
#include "Variable.hpp"

using namespace eddic;
    
ast::DebugVisitor::DebugVisitor() : level(0) {}

std::string ast::DebugVisitor::indent() const {
    std::string acc = "";
    for(int i = 0; i < level; ++i){
        acc += "\t";
    }
    return acc;
}

void ast::DebugVisitor::operator()(ast::SourceFile& program) const {
    std::cout << indent() << "SourceFile" << std::endl; 

    ++level;

    visit_each(*this, program.Content->blocks);
}

void ast::DebugVisitor::operator()(ast::Import& import) const {
    std::cout << indent() << "include \"" << import.file << "\"" << std::endl;
}

void ast::DebugVisitor::operator()(ast::StandardImport& import) const {
    std::cout << indent() << "include <" << import.header << ">" << std::endl;
}

template<typename Visitor, typename Container>
void print_each_sub(Visitor& visitor, Container& container){
    visitor.level++;
    visit_each(visitor, container);    
    visitor.level--;
}

template<typename Visitor, typename Container>
void print_sub(Visitor& visitor, Container& container){
    visitor.level++;
    visit(visitor, container);    
    visitor.level--;
}

void ast::DebugVisitor::operator()(ast::FunctionDeclaration& declaration) const {
    std::cout << indent() << "Function " << declaration.Content->functionName << std::endl; 
    print_each_sub(*this, declaration.Content->instructions);
}

void ast::DebugVisitor::operator()(ast::GlobalVariableDeclaration&) const {
    std::cout << indent() << "Global Variable" << std::endl; 
}

void ast::DebugVisitor::operator()(ast::GlobalArrayDeclaration&) const {
    std::cout << indent() << "Global Array" << std::endl; 
}

void ast::DebugVisitor::operator()(ast::For& for_) const {
    std::cout << indent() << "For" << std::endl; 
    print_each_sub(*this, for_.Content->instructions);
}

void ast::DebugVisitor::operator()(ast::Foreach& for_) const {
    std::cout << indent() << "Foreach" << std::endl; 
    print_each_sub(*this, for_.Content->instructions);
}

void ast::DebugVisitor::operator()(ast::ForeachIn& for_) const {
    std::cout << indent() << "Foreach in " << std::endl; 
    print_each_sub(*this, for_.Content->instructions);
}

void ast::DebugVisitor::operator()(ast::While& while_) const {
    std::cout << indent() << "While" << std::endl; 
    std::cout << indent() << "Condition:" << std::endl;
    print_sub(*this, while_.Content->condition);
    print_each_sub(*this, while_.Content->instructions);
}

void ast::DebugVisitor::operator()(ast::DoWhile& while_) const {
    std::cout << indent() << "Do while" << std::endl; 
    std::cout << indent() << "Condition:" << std::endl;
    print_sub(*this, while_.Content->condition);
    print_each_sub(*this, while_.Content->instructions);
}

void ast::DebugVisitor::operator()(ast::Swap&) const {
    std::cout << indent() << "Swap" << std::endl; 
}

void ast::DebugVisitor::operator()(ast::If& if_) const {
    std::cout << indent() << "If" << std::endl; 
    std::cout << indent() << "Condition:" << std::endl;
    print_sub(*this, if_.Content->condition);
    std::cout << indent() << "Body:" << std::endl;
    print_each_sub(*this, if_.Content->instructions);
}

void ast::DebugVisitor::operator()(ast::FunctionCall& call) const {
    std::cout << indent() << "FunctionCall " << call.Content->functionName << std::endl; 
    print_each_sub(*this, call.Content->values);
}

void ast::DebugVisitor::operator()(ast::BuiltinOperator& builtin) const {
    std::cout << indent() << "Builtin Operator " << (int) builtin.Content->type << std::endl; 
    print_each_sub(*this, builtin.Content->values);
}

void ast::DebugVisitor::operator()(ast::VariableDeclaration& declaration) const {
    std::cout << indent() << "Variable declaration" << std::endl; 

    if(declaration.Content->value){
        print_sub(*this, *declaration.Content->value);
    }
}

void ast::DebugVisitor::operator()(ast::ArrayDeclaration&) const {
    std::cout << indent() << "Array declaration" << std::endl; 
}

void ast::DebugVisitor::operator()(ast::SuffixOperation& op) const {
    std::cout << indent() << op.Content->variableName << "(suffix)" << (int)op.Content->op << std::endl; 
}

void ast::DebugVisitor::operator()(ast::PrefixOperation& op) const {
    std::cout << indent() << (int)op.Content->op << "(prefix)" << op.Content->variableName << std::endl; 
}

void ast::DebugVisitor::operator()(ast::Assignment& assign) const {
    std::cout << indent() << "Variable assignment" << std::endl; 
    print_sub(*this, assign.Content->value);
}

void ast::DebugVisitor::operator()(ast::CompoundAssignment& assign) const {
    std::cout << indent() << "Compound variable assignment [operator = " << (int) assign.Content->op << " ]" << std::endl; 
    print_sub(*this, assign.Content->value);
}

void ast::DebugVisitor::operator()(ast::Return& return_) const {
    std::cout << indent() << "Function return" << std::endl; 
    print_sub(*this, return_.Content->value);
}

void ast::DebugVisitor::operator()(ast::ArrayAssignment& assign) const {
    std::cout << indent() << "Array assignment" << std::endl; 
    print_sub(*this, assign.Content->value);
}

void ast::DebugVisitor::operator()(ast::Litteral& litteral) const {
    std::cout << indent() << "Litteral [" << litteral.value << "]" << std::endl; 
}

void ast::DebugVisitor::operator()(ast::Integer& integer) const {
    std::cout << indent() << "Integer [" << integer.value << "]" << std::endl; 
}

void ast::DebugVisitor::operator()(ast::True&) const {
    std::cout << indent() << "true" << std::endl; 
}

void ast::DebugVisitor::operator()(ast::False&) const {
    std::cout << indent() << "false" << std::endl; 
}

void ast::DebugVisitor::operator()(ast::VariableValue& value) const {
    std::cout << indent() << "Variable [" << value.Content->var->name()  << "]" << std::endl; 
}

void ast::DebugVisitor::operator()(ast::ArrayValue&) const {
    std::cout << indent() << "Array value" << std::endl; 
}

void ast::DebugVisitor::operator()(ast::ComposedValue& value) const {
    std::cout << indent() << "Composed value [" << value.Content->operations.size() << "]" << std::endl; 
    ++level;
    visit(*this, value.Content->first);
    for(auto& operation : value.Content->operations){
        std::cout << indent() << (int) operation.get<0>() << std::endl;
        visit(*this, operation.get<1>());
    }
    --level;
}

void ast::DebugVisitor::operator()(ast::Minus& value) const {
    std::cout << indent() << "Unary +" << std::endl; 
    print_sub(*this, value.Content->value);
}

void ast::DebugVisitor::operator()(ast::Plus& value) const {
    std::cout << indent() << "Unary -" << std::endl; 
    print_sub(*this, value.Content->value);
}
