//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Integer.hpp"

#include "AssemblyFileWriter.hpp"

using namespace eddic;
        
Integer::Integer(std::shared_ptr<Context> context, const Tok token, int value) : Value(context, token), m_value(value) {
    m_type = Type::INT;
}

void Integer::write(AssemblyFileWriter& writer) {
    writer.stream() << "pushl $" << m_value << std::endl;
}

bool Integer::isConstant() {
    return true;
}

int Integer::getIntValue() {
    return m_value;
}
