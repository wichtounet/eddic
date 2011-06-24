//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Operators.h"
#include "Options.h"

#include <cassert>

using std::string;

//Checks

void BinaryOperator::checkVariables(Variables& variables) throw (CompilerException){
	NodeIterator it = begin();

	Value* lhs = dynamic_cast<Value*>(*it++);
	Value* rhs = dynamic_cast<Value*>(*it);

	lhs->checkVariables(variables);
	rhs->checkVariables(variables);

	m_type = checkTypes(lhs->type(), rhs->type());
}

Type Addition::checkTypes(Type left, Type right) throw (CompilerException){
	if(left != right){
		throw new CompilerException("Can only add two values of the same type");
	}

	return left;
}

Type Subtraction::checkTypes(Type left, Type right) throw (CompilerException){
	if(left != right || left != INT){
		throw new CompilerException("Can only subtract two integers");
	}

	return left;
}

//Writes

void Addition::write(ByteCodeFileWriter& writer){
	lhs()->write(writer);
	rhs()->write(writer);

	if(m_type == INT){
		writer.writeSimpleCall(IADD);
	} else {
		writer.writeSimpleCall(SADD);
	}
}

void Subtraction::write(ByteCodeFileWriter& writer){
	lhs()->write(writer);
	rhs()->write(writer);

	writer.writeSimpleCall(ISUB);
}

//Constantness

bool BinaryOperator::isConstant(){
	return lhs()->isConstant() && rhs()->isConstant();
}

//Values

int BinaryOperator::getIntValue(){
	if(type() != INT){
		throw "Invalid type";
	}

	if(!isConstant()){
		throw "Not a constant";
	}

	return compute(lhs()->getIntValue(), rhs()->getIntValue());
}

string BinaryOperator::getStringValue(){
	if(type() != STRING){
		throw "Invalid type"; 
	}

	if(!isConstant()){
		throw "Not a constant";
	}
	
	return compute(lhs()->getStringValue(), rhs()->getStringValue());
}

int BinaryOperator::compute(int left, int right){
	throw "Invalid type"; 
}

string BinaryOperator::compute(string left, string right){
	throw "Invalid type"; 
}

int Addition::compute(int left, int right){
	return left + right;
}

string Addition::compute(string left, string right){
	return left + right;
}

int Subtraction::compute(int left, int right){
	return left - right;
}

//Optimizations

void Addition::optimize(){
	//If both of the values are constant, we'll replace the addition with the value of the addition
	if(isConstant()){
		if(type() == INT){
			if(Options::isSet(OPTIMIZE_INTEGERS) || Options::isSet(OPTIMIZE_ALL)){
				Value* value = new Integer(getIntValue());

				parent->replace(this, value);
			}
		} else if(type() == STRING){
			if(Options::isSet(OPTIMIZE_STRINGS) || Options::isSet(OPTIMIZE_ALL)){
				//No optimization at this time
			}
		}
	}
	
	lhs()->optimize();
	rhs()->optimize();	
}

void Subtraction::optimize(){
	//If both of the values are constant, we'll replace the subtraction with the value of the subtraction
	if(isConstant()){
		if(Options::isSet(OPTIMIZE_INTEGERS) || Options::isSet(OPTIMIZE_ALL)){
			Value* value = new Integer(getIntValue());

			parent->replace(this, value);
		}
	}
	
	lhs()->optimize();
	rhs()->optimize();	
}

//Utilities

Value* BinaryOperator::lhs(){
	return dynamic_cast<Value*>(*begin());
}

Value* BinaryOperator::rhs(){
	return dynamic_cast<Value*>(*++begin());
}
