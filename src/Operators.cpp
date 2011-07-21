//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Operators.hpp"
#include "Options.hpp"
#include "AssemblyFileWriter.hpp"
#include "Variables.hpp"

#include <cassert>

using std::string;

using namespace eddic;

//Checks

void BinaryOperator::checkVariables(Variables& variables) {
    lhs->checkVariables(variables);
    rhs->checkVariables(variables);

    m_type = checkTypes(lhs->type(), rhs->type());
}

void BinaryOperator::checkStrings(StringPool& pool) {
    lhs->checkStrings(pool);
    rhs->checkStrings(pool);
}

Type BinaryOperator::checkTypes(Type left, Type right) {
    if (left != right || left != INT) {
        throw CompilerException("Can only compute two integers");
    }

    return left;
}

Type Addition::checkTypes(Type left, Type right) {
    if (left != right) {
        throw CompilerException("Can only add two values of the same type");
    }

    return left;
}

//Writes

void Addition::write(AssemblyFileWriter& writer) {
    lhs->write(writer);
    rhs->write(writer);

    static int labelCopy = 0;

    if (m_type == INT) {
        writer.stream() << "movl (%esp), %eax" << std::endl;
        writer.stream() << "movl 4(%esp), %ecx" << std::endl;
        writer.stream() << "addl %ecx, %eax" << std::endl;
        writer.stream() << "addl $8, %esp" << std::endl;
        writer.stream() << "pushl %eax" << std::endl;
    } else {
        writer.stream() << "movl 8(%esp), %edx" << std::endl;
        writer.stream() << "movl (%esp), %ecx" << std::endl;
        writer.stream() << "addl %ecx, %edx" << std::endl;

        writer.stream() << "pushl %edx" << std::endl;
        writer.stream() << "call malloc" << std::endl;
        writer.stream() << "addl $4, %esp" << std::endl;

        writer.stream() << "movl %eax, -4(%esp)" << std::endl;
        writer.stream() << "movl %eax, %ecx" << std::endl;
        writer.stream() << "movl $0, %eax" << std::endl;

        writer.stream() << "movl 8(%esp), %ebx" << std::endl;
        writer.stream() << "movl 12(%esp), %edx" << std::endl;

        writer.stream() << "copy" << labelCopy << ":" << std::endl;
        writer.stream() << "cmpl $0, %ebx" << std::endl;
        writer.stream() << "je end" << labelCopy  << std::endl;
        writer.stream() << "movb (%edx), %al" << std::endl;
        writer.stream() << "movb %al, (%ecx)" << std::endl;
        writer.stream() << "addl $1, %ecx" << std::endl;
        writer.stream() << "addl $1, %edx" << std::endl;
        writer.stream() << "subl $1, %ebx" << std::endl;
        writer.stream() << "jmp copy" << labelCopy << std::endl;
        writer.stream() << "end" << labelCopy << ":" << std::endl;

        labelCopy++;

        writer.stream() << "movl (%esp), %ebx" << std::endl;
        writer.stream() << "movl 4(%esp), %edx" << std::endl;

        writer.stream() << "copy" << labelCopy << ":" << std::endl;
        writer.stream() << "cmpl $0, %ebx" << std::endl;
        writer.stream() << "je end" << labelCopy  << std::endl;
        writer.stream() << "movb (%edx), %al" << std::endl;
        writer.stream() << "movb %al, (%ecx)" << std::endl;
        writer.stream() << "addl $1, %ecx" << std::endl;
        writer.stream() << "addl $1, %edx" << std::endl;
        writer.stream() << "subl $1, %ebx" << std::endl;
        writer.stream() << "jmp copy" << labelCopy << std::endl;
        writer.stream() << "end" << labelCopy << ":" << std::endl;

        writer.stream() << "movl 8(%esp), %edx" << std::endl;
        writer.stream() << "movl (%esp), %ecx" << std::endl;
        writer.stream() << "addl %ecx, %edx" << std::endl;

        labelCopy++;

        writer.stream() << "movl -4(%esp), %eax" << std::endl;

        writer.stream() << "addl $16, %esp" << std::endl;

        writer.stream() << "pushl %eax" << std::endl;
        writer.stream() << "pushl %edx" << std::endl;
    }
}

void Subtraction::write(AssemblyFileWriter& writer) {
    lhs->write(writer);
    rhs->write(writer);

    writer.stream() << "movl (%esp), %ecx" << std::endl;
    writer.stream() << "movl 4(%esp), %eax" << std::endl;
    writer.stream() << "subl %ecx, %eax" << std::endl;
    writer.stream() << "addl $8, %esp" << std::endl;
    writer.stream() << "pushl %eax" << std::endl;
}

void Multiplication::write(AssemblyFileWriter& writer) {
    lhs->write(writer);
    rhs->write(writer);

    writer.stream() << "movl (%esp), %eax" << std::endl;
    writer.stream() << "movl 4(%esp), %ecx" << std::endl;
    writer.stream() << "mull %ecx" << std::endl;
    writer.stream() << "addl $8, %esp" << std::endl;
    writer.stream() << "pushl %eax" << std::endl;
}

void Division::write(AssemblyFileWriter& writer) {
    lhs->write(writer);
    rhs->write(writer);

    writer.stream() << "movl (%esp), %ecx" << std::endl;
    writer.stream() << "movl 4(%esp), %eax" << std::endl;
    writer.stream() << "movl $0, %edx" << std::endl;
    writer.stream() << "divl %ecx" << std::endl;
    writer.stream() << "addl $8, %esp" << std::endl;
    writer.stream() << "pushl %eax" << std::endl;
}

void Modulo::write(AssemblyFileWriter& writer) {
    lhs->write(writer);
    rhs->write(writer);

    writer.stream() << "movl (%esp), %ecx" << std::endl;
    writer.stream() << "movl 4(%esp), %eax" << std::endl;
    writer.stream() << "movl $0, %edx" << std::endl;
    writer.stream() << "divl %ecx" << std::endl;
    writer.stream() << "addl $8, %esp" << std::endl;
    writer.stream() << "pushl %edx" << std::endl;
}

//Constantness

bool BinaryOperator::isConstant() {
    return lhs->isConstant() && rhs->isConstant();
}

//Values

int BinaryOperator::getIntValue() {
    if (type() != INT) {
        throw "Invalid type";
    }

    if (!isConstant()) {
        throw "Not a constant";
    }

    return compute(lhs->getIntValue(), rhs->getIntValue());
}

string BinaryOperator::getStringValue() {
    if (type() != STRING) {
        throw "Invalid type";
    }

    if (!isConstant()) {
        throw "Not a constant";
    }

    return compute(lhs->getStringValue(), rhs->getStringValue());
}

int BinaryOperator::compute(int left, int right) {
    throw "Invalid type";
}

string BinaryOperator::compute(const std::string& left, const std::string& right) {
    throw "Invalid type";
}

int Addition::compute(int left, int right) {
    return left + right;
}

string Addition::compute(const std::string& left, const std::string& right) {
    return left + right;
}

int Subtraction::compute(int left, int right) {
    return left - right;
}

int Multiplication::compute(int left, int right) {
    return left * right;
}

int Division::compute(int left, int right) {
    return left / right;
}

int Modulo::compute(int left, int right) {
    return left % right;
}

//Optimizations

void BinaryOperator::optimize() {
    if (isConstant()) {
        if (Options::isSet(OPTIMIZE_INTEGERS)) {
            Value* value = new Integer(getIntValue());

            parent->replace(this, value);
        }
    }

    lhs->optimize();
    rhs->optimize();
}

void Addition::optimize() {
    if (isConstant()) {
        if (type() == INT) {
            if (Options::isSet(OPTIMIZE_INTEGERS)) {
                Value* value = new Integer(getIntValue());

                parent->replace(this, value);
            }
        } else if (type() == STRING) {
            if (Options::isSet(OPTIMIZE_STRINGS)) {
                //No optimization at this time
            }
        }
    }

    lhs->optimize();
    rhs->optimize();
}
