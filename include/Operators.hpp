//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef OPERATORS_H
#define OPERATORS_H

#include <memory>

#include "Nodes.hpp"

namespace eddic {

class BinaryOperator : public Value {
    protected:
        std::shared_ptr<Value> lhs;
        std::shared_ptr<Value> rhs;

        BinaryOperator(std::shared_ptr<Context> context, const Token* token, std::shared_ptr<Value> l, std::shared_ptr<Value> r) : Value(context, token), lhs(l), rhs(r) {}

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
        Addition(std::shared_ptr<Context> context, const Token* token, std::shared_ptr<Value> lhs, std::shared_ptr<Value> rhs) : BinaryOperator(context, token, lhs, rhs) {}

        void write(AssemblyFileWriter& writer);
        void optimize();

        Type checkTypes(Type left, Type right);
        std::string compute(const std::string& left, const std::string& right);
        int compute(int left, int right);
};

class Subtraction : public BinaryOperator {
    public:
        Subtraction(std::shared_ptr<Context> context, const Token* token, std::shared_ptr<Value> lhs, std::shared_ptr<Value> rhs) : BinaryOperator(context, token, lhs, rhs) {}

        void write(AssemblyFileWriter& writer);

        int compute(int left, int right);
};

class Multiplication : public BinaryOperator {
    public:
        Multiplication(std::shared_ptr<Context> context, const Token* token, std::shared_ptr<Value> lhs, std::shared_ptr<Value> rhs) : BinaryOperator(context, token, lhs, rhs) {}

        void write(AssemblyFileWriter& writer);

        int compute(int left, int right);
};

class Division : public BinaryOperator {
    public:
        Division(std::shared_ptr<Context> context, const Token* token, std::shared_ptr<Value> lhs, std::shared_ptr<Value> rhs) : BinaryOperator(context, token, lhs, rhs) {}

        void write(AssemblyFileWriter& writer);

        int compute(int left, int right);
};

class Modulo : public BinaryOperator {
    public:
        Modulo(std::shared_ptr<Context> context, const Token* token, std::shared_ptr<Value> lhs, std::shared_ptr<Value> rhs) : BinaryOperator(context, token, lhs, rhs) {}

        void write(AssemblyFileWriter& writer);

        int compute(int left, int right);
};

} //end of eddic

#endif
