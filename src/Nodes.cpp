//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Nodes.hpp"
#include "StringPool.hpp"
#include "Options.hpp"
#include "ByteCodeFileWriter.hpp"
#include "Variables.hpp"

#include <cassert>

using std::string;
using std::endl;

using namespace eddic;

void Declaration::checkVariables(Variables& variables) throw (CompilerException) {
    if(variables.exists(m_variable)) {
        throw CompilerException("Variable has already been declared");
    }

    Variable* var = variables.create(m_variable, m_type);

    m_index = var->index();

    value->checkVariables(variables);

    if(value->type() != m_type) {
        throw CompilerException("Incompatible type");
    }
}

void Declaration::checkStrings(StringPool& pool) {
    value->checkStrings(pool);
}

void Assignment::checkVariables(Variables& variables) throw (CompilerException) {
    if(!variables.exists(m_variable)) {
        throw CompilerException("Variable has not  been declared");
    }

    Variable* var = variables.find(m_variable);

    m_index = var->index();

    value->checkVariables(variables);

    if(value->type() != var->type()) {
        throw CompilerException("Incompatible type");
    }
}

void Swap::checkVariables(Variables& variables) throw (CompilerException) {
    if(m_lhs == m_rhs){
        throw CompilerException("Cannot swap a variable with itself");
    }
    
    if(!variables.exists(m_lhs) || !variables.exists(m_rhs)) {
        throw CompilerException("Variable has not been declared");
    }

    Variable* lhs_var = variables.find(m_lhs);
    Variable* rhs_var = variables.find(m_rhs);

    m_lhs_index = lhs_var->index();
    m_rhs_index = rhs_var->index();

    if(lhs_var->type() != rhs_var->type()) {
        throw CompilerException("Incompatible type");
    }

    m_type = lhs_var->type();
}

void Assignment::checkStrings(StringPool& pool) {
    value->checkStrings(pool);
}

void VariableValue::checkVariables(Variables& variables) throw (CompilerException) {
    if(!variables.exists(m_variable)) {
        throw CompilerException("Variable has not been declared");
    }

    Variable* variable = variables.find(m_variable);

    m_type = variable->type();
    m_index = variable->index();
}

void Litteral::checkStrings(StringPool& pool) {
    m_label = pool.label(m_litteral);
}

void Declaration::write(ByteCodeFileWriter& writer) {
    value->write(writer);

    switch(m_type) {
        case INT:
            writer.stream() << "movl (%esp), %eax" << endl;
            writer.stream() << "movl %eax, VI" << m_index << endl;
            writer.stream() << "addl $4, %esp" << endl;

            break;
        case STRING:
            writer.stream() << "movl (%esp), %eax" << endl;
            writer.stream() << "movl 4(%esp), %ebx" << endl;
            writer.stream() << "addl $8, %esp" << endl;
            
            writer.stream() << "movl %eax, VS" << m_index << "_l" << endl;
            writer.stream() << "movl %ebx, VS" << m_index << endl;

            break;
    }
}

void Assignment::write(ByteCodeFileWriter& writer) {
    value->write(writer);

    switch(value->type()) {
        case INT:
            writer.stream() << "movl (%esp), %eax" << std::endl;
            writer.stream() << "movl %eax, VI" << m_index << "" << std::endl;
            writer.stream() << "addl $4, %esp" << std::endl;

            break;
        case STRING:
            writer.stream() << "movl (%esp), %eax" << endl;
            writer.stream() << "movl 4(%esp), %ebx" << endl;
            writer.stream() << "addl $8, %esp" << endl;
            
            writer.stream() << "movl %eax, VS" << m_index << "_l" << endl;
            writer.stream() << "movl %ebx, VS" << m_index << endl;

            break;
    }
}

void Swap::write(ByteCodeFileWriter& writer) {
    switch(m_type){
        case INT:
            writer.stream() << "movl VI" << m_lhs_index << ", %eax" << endl;
            writer.stream() << "movl VI" << m_rhs_index << ", %ebx" << endl;
            writer.stream() << "movl %eax, VI" << m_rhs_index << endl;
            writer.stream() << "movl %ebx, VI" << m_lhs_index << endl;

            break;
        case STRING:
            writer.stream() << "movl VS" << m_lhs_index << ", %eax" << endl;
            writer.stream() << "movl VS" << m_rhs_index << ", %ebx" << endl;
            writer.stream() << "movl %eax, VS" << m_rhs_index << endl;
            writer.stream() << "movl %ebx, VS" << m_lhs_index << endl;
            
            writer.stream() << "movl VS" << m_lhs_index << "_l, %eax" << endl;
            writer.stream() << "movl VS" << m_rhs_index << "_l, %ebx" << endl;
            writer.stream() << "movl %eax, VS" << m_rhs_index << "_l" << endl;
            writer.stream() << "movl %ebx, VS" << m_lhs_index << "_l" << endl;
        
            break;
    }
}

void Print::write(ByteCodeFileWriter& writer) {
    value->write(writer);

    switch(value->type()) {
        case INT:
            writer.stream() << "call print_integer" << endl;
            writer.stream() << "addl $4, %esp" << endl;

            break;
        case STRING:
            writer.stream() << "call print_string" << endl;
            writer.stream() << "addl $8, %esp" << endl;

            break;
    }
}

void Println::write(ByteCodeFileWriter& writer){
    value->write(writer);

    switch(value->type()) {
        case INT:
            writer.stream() << "call print_integer" << endl;
            writer.stream() << "addl $4, %esp" << endl;

            break;
        case STRING:
            writer.stream() << "call print_string" << endl;
            writer.stream() << "addl $8, %esp" << endl;

            break;
    }

    writer.stream() << "call print_line" << endl;
}

void Print::checkStrings(StringPool& pool) {
    value->checkStrings(pool);
}

void Print::checkVariables(Variables& variables) throw (CompilerException) {
    value->checkVariables(variables);
}

void Integer::write(ByteCodeFileWriter& writer) {
    writer.stream() << "pushl $" << m_value << std::endl;
}

void VariableValue::write(ByteCodeFileWriter& writer) {
    switch(m_type) {
        case INT:
            writer.stream() << "pushl VI" << m_index << std::endl;

            break;
        case STRING:
            writer.stream() << "pushl VS" << m_index << endl;
            writer.stream() << "pushl VS" << m_index << "_l" << std::endl;

            break;
    }
}

void Litteral::write(ByteCodeFileWriter& writer) {
    writer.stream() << "pushl $" << m_label << std::endl;
    writer.stream() << "pushl $" << (m_litteral.size() - 2) << std::endl;
}

//Constantness

bool Value::isConstant() {
    return false;
}

bool Litteral::isConstant() {
    return true;
}

bool Integer::isConstant() {
    return true;
}

bool VariableValue::isConstant() {
    return false;
}

//Values

string Value::getStringValue() {
    throw "Not constant";
}

int Value::getIntValue() {
    throw "Not constant";
}

int Integer::getIntValue() {
    return m_value;
}

string Litteral::getStringValue() {
    return m_litteral;
}
