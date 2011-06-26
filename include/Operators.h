//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef OPERATORS_H
#define OPERATORS_H

#include "Nodes.h"

class BinaryOperator : public Value {
	protected:
		Value* lhs;
		Value* rhs;

		BinaryOperator(Value* lhs, Value* rhs);

		bool isConstant();
		void checkVariables(Variables& variables) throw (CompilerException);
		std::string getStringValue();
		int getIntValue();
		void optimize();

		virtual Type checkTypes(Type left, Type right) throw (CompilerException);
		virtual std::string compute(std::string left, std::string right);
		virtual int compute(int left, int right);
};

class Addition : public BinaryOperator {
	public:
		Addition(Value* lhs, Value* rhs) : BinaryOperator(lhs, rhs) {}

		void write(ByteCodeFileWriter& writer); 
		void optimize();

		Type checkTypes(Type left, Type right) throw (CompilerException);
		std::string compute(std::string left, std::string right);
		int compute(int left, int right);
};

class Subtraction : public BinaryOperator {
	public:
		Subtraction(Value* lhs, Value* rhs) : BinaryOperator(lhs, rhs) {}
		
		void write(ByteCodeFileWriter& writer); 

		int compute(int left, int right);
};

class Multiplication : public BinaryOperator {
	public:
		Multiplication(Value* lhs, Value* rhs) : BinaryOperator(lhs, rhs) {}
		
		void write(ByteCodeFileWriter& writer); 

		int compute(int left, int right);
};

class Division : public BinaryOperator {
	public:
		Division(Value* lhs, Value* rhs) : BinaryOperator(lhs, rhs) {}
		
		void write(ByteCodeFileWriter& writer); 

		int compute(int left, int right);
};

class Modulo : public BinaryOperator {
	public:
		Modulo(Value* lhs, Value* rhs) : BinaryOperator(lhs, rhs) {}
		
		void write(ByteCodeFileWriter& writer); 

		int compute(int left, int right);
};

#endif
