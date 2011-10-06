//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "Litteral.hpp"

#include "StringPool.hpp"
#include "AssemblyFileWriter.hpp"

using namespace eddic;

void Litteral::checkStrings(StringPool& pool) {
    m_label = pool.label(m_litteral);
}

void Litteral::write(AssemblyFileWriter& writer) {
    writer.stream() << "pushl $" << getStringLabel() << std::endl;
    writer.stream() << "pushl $" << getStringSize() << std::endl;
}

bool Litteral::isConstant() {
    return true;
}

std::string Litteral::getStringValue() {
    return m_litteral;
}  

std::string Litteral::getStringLabel(){
    return m_label;
}

int Litteral::getStringSize(){
    return m_litteral.size() - 2;
}
