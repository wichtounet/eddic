//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Nodes.h"
#include "StringPool.h"

#include <cassert>

using std::string;

void Declaration::checkVariables(Variables& variables) throw (CompilerException){
	if(variables.exists(m_variable)){
		throw CompilerException("Variable has already been declared");
	}

	Variable* var = variables.create(m_variable, m_type);

	m_index = var->index();

	Value* value = dynamic_cast<Value*>(*begin());

	value->checkVariables(variables);

	if(value->type() != m_type){
		throw CompilerException("Incompatible type");
	}
}

void Assignment::checkVariables(Variables& variables) throw (CompilerException){
	if(!variables.exists(m_variable)){
		throw CompilerException("Variable has not  been declared");
	}

	Variable* var = variables.find(m_variable);

	m_index = var->index();
	
	Value* value = dynamic_cast<Value*>(*begin());

	value->checkVariables(variables);

	if(value->type() != var->type()){
		throw CompilerException("Incompatible type");
	}
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
	m_index = pool.index(m_litteral);	
}

void Declaration::write(ByteCodeFileWriter& writer){
	(*begin())->write(writer);	
	
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
	(*begin())->write(writer);	
	
	Value* value = dynamic_cast<Value*>(*begin());
	
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
	(*begin())->write(writer);	
	
	Value* value = dynamic_cast<Value*>(*begin());
	
	switch(value->type()){
		case INT:
			writer.writeSimpleCall(PRINTI);

			break;
		case STRING:
			writer.writeSimpleCall(PRINTS);

			break;
	}
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
	writer.writeOneOperandCall(LDCS, m_index);
}

void Addition::checkVariables(Variables& variables) throw (CompilerException){
	NodeIterator it = begin();

	Value* lhs = dynamic_cast<Value*>(*it++);
	Value* rhs = dynamic_cast<Value*>(*it);

	lhs->checkVariables(variables);
	rhs->checkVariables(variables);

	if(lhs->type() != rhs->type()){
		throw new CompilerException("Can only add two values of the same type");
	}

	m_type = lhs->type();
}

void Addition::write(ByteCodeFileWriter& writer){
	lhs()->write(writer);
	rhs()->write(writer);

	if(m_type == INT){
		writer.writeSimpleCall(IADD);
	} else {
		writer.writeSimpleCall(SADD);
	}
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

bool Addition::isConstant(){
	return lhs()->isConstant() && rhs()->isConstant();
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

int Addition::getIntValue(){
	if(type() != INT){
		throw "Invalid type";
	}

	if(!isConstant()){
		throw "Not a constant";
	}

	return lhs()->getIntValue() + rhs()->getIntValue();	
}

string Addition::getStringValue(){
	if(type() != STRING){
		throw "Invalid type"; 
	}

	if(!isConstant()){
		throw "Not a constant";
	}
	
	return lhs()->getStringValue() + rhs()->getStringValue();
}

//Optimizations

void Addition::optimize(){
	//If both of the values are constant, we'll replace the addition with the value of the addition
	if(isConstant()){
		NodeIterator it = begin();

		if(type() == INT){
			Value* value = new Integer(getIntValue());
			
			parent->replace(this, value);
		} else if(type() == STRING){
			//No optimization at this time
		}
	}
	
	lhs()->optimize();
	rhs()->optimize();	
}

//Utilities

Value* Addition::lhs(){
	return dynamic_cast<Value*>(*begin());
}

Value* Addition::rhs(){
	return dynamic_cast<Value*>(*++begin());
}
