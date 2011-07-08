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

void Declaration::checkVariables(Variables& variables) throw (CompilerException){
	if(variables.exists(m_variable)){
		throw CompilerException("Variable has already been declared");
	}

	Variable* var = variables.create(m_variable, m_type);

	m_index = var->index();

	value->checkVariables(variables);

	if(value->type() != m_type){
		throw CompilerException("Incompatible type");
	}
}

void Declaration::checkStrings(StringPool& pool){
	value->checkStrings(pool);
}

void Assignment::checkVariables(Variables& variables) throw (CompilerException){
	if(!variables.exists(m_variable)){
		throw CompilerException("Variable has not  been declared");
	}

	Variable* var = variables.find(m_variable);

	m_index = var->index();
	
	value->checkVariables(variables);

	if(value->type() != var->type()){
		throw CompilerException("Incompatible type");
	}
}

void Assignment::checkStrings(StringPool& pool){
	value->checkStrings(pool);
}

void VariableValue::checkVariables(Variables& variables) throw (CompilerException){
	if(!variables.exists(m_variable)){
		throw CompilerException("Variable has not been declared");
	}

	Variable* variable = variables.find(m_variable);

	m_type = variable->type();
	m_index = variable->index();
}

void Litteral::checkStrings(StringPool& pool){
	m_label = pool.label(m_litteral);	
}

void Declaration::write(ByteCodeFileWriter& writer){
	value->write(writer);	
	
	switch(m_type){
		case INT:
			writer.writeOneOperandCall(ISTORE, m_index);

			break;
		case STRING:
			writer.writeOneOperandCall(SSTORE, m_index);

			break;
	}
}

void Assignment::write(ByteCodeFileWriter& writer){
	value->write(writer);	
	
	switch(value->type()){
		case INT:
			writer.writeOneOperandCall(ISTORE, m_index);

			break;
		case STRING:
			writer.writeOneOperandCall(SSTORE, m_index);

			break;
	}
}

void Print::write(ByteCodeFileWriter& writer){
	value->write(writer);	
	
	switch(value->type()){
		case INT:
			writer.nativeWrite("call print_integer");
			writer.nativeWrite("addl $4, %esp");

			break;
		case STRING:
			writer.nativeWrite("call print_string");
			writer.nativeWrite("addl $8, %esp");

			break;
	}
}

void Print::checkStrings(StringPool& pool){
	value->checkStrings(pool);
}

void Print::checkVariables(Variables& variables) throw (CompilerException) {
	value->checkVariables(variables);
}

void Integer::write(ByteCodeFileWriter& writer){
	writer.writeOneOperandCall(LDCI, m_value);	
}

void VariableValue::write(ByteCodeFileWriter& writer){
	switch(m_type){
		case INT:
			writer.writeOneOperandCall(ILOAD, m_index);

			break;
		case STRING:
			writer.writeOneOperandCall(SLOAD, m_index);

			break;
	}
}

void Litteral::write(ByteCodeFileWriter& writer){
	writer.stream() << "pushl $" << m_label << std::endl;
	writer.stream() << "pushl $" << m_litteral.size() << std::endl;
}

//Constantness

bool Value::isConstant(){
	return false;
}

bool Litteral::isConstant(){
	return true;
}

bool Integer::isConstant(){
	return true;
}

bool VariableValue::isConstant(){
	return false;
}

//Values

string Value::getStringValue(){
	throw "Not constant";
}

int Value::getIntValue(){
	throw "Not constant";
}

int Integer::getIntValue(){
	return m_value;
}

string Litteral::getStringValue(){
	return m_litteral;
}
