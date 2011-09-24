//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef OPERATORS_H
#define OPERATORS_H

#include "Nodes.hpp"

namespace eddic {

class BinaryOperator : public Value {
    protected:
        Value* lhs;
        Value* rhs;

        BinaryOperator(Context* context, const Token* token, Value* l, Value* r) : Value(context, token), lhs(l), rhs(r) {}

        virtual ~BinaryOperator() {
            delete lhs;
            delete rhs;
        }

        bool isConstant();
        void checkVariables();
        void checkStrings(StringPool& pool);
        std::string getStringValue();
        int getIntValue();
        void optimize();

        virtual Type checkTypes(Type left, Type right);
        virtual std::string compute(const std::string& left, const std::string& right);
        virtual int compute(int left, int right);
};

class Addition : public BinaryOperator {
    public:
        Addition(Context* context, const Token* token, Value* lhs, Value* rhs) : BinaryOperator(context, token, lhs, rhs) {}

        void write(AssemblyFileWriter& writer);
        void optimize();

        Type checkTypes(Type left, Type right);
        std::string compute(const std::string& left, const std::string& right);
        int compute(int left, int right);
};

class Subtraction : public BinaryOperator {
    public:
        Subtraction(Context* context, const Token* token, Value* lhs, Value* rhs) : BinaryOperator(context, token, lhs, rhs) {}

        void write(AssemblyFileWriter& writer);

        int compute(int left, int right);
};

class Multiplication : public BinaryOperator {
    public:
        Multiplication(Context* context, const Token* token, Value* lhs, Value* rhs) : BinaryOperator(context, token, lhs, rhs) {}

        void write(AssemblyFileWriter& writer);

        int compute(int left, int right);
};

class Division : public BinaryOperator {
    public:
        Division(Context* context, const Token* token, Value* lhs, Value* rhs) : BinaryOperator(context, token, lhs, rhs) {}

        void write(AssemblyFileWriter& writer);

        int compute(int left, int right);
};

class Modulo : public BinaryOperator {
    public:
        Modulo(Context* context, const Token* token, Value* lhs, Value* rhs) : BinaryOperator(context, token, lhs, rhs) {}

        void write(AssemblyFileWriter& writer);

        int compute(int left, int right);
};

} //end of eddic

#endif
