//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Modulo.hpp"
#include "AssemblyFileWriter.hpp"

using namespace eddic;
        
Modulo::Modulo(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, std::shared_ptr<Value> lhs, std::shared_ptr<Value> rhs) : BinaryOperator(context, token, lhs, rhs) {}

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

int Modulo::compute(int left, int right) {
    return left % right;
}
