//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/Push.hpp"

#include "il/Operand.hpp"
#include "AssemblyFileWriter.hpp"

using namespace eddic;

Push::Push(std::shared_ptr<Operand> operand) : m_operand(operand) {}

void Push::write(AssemblyFileWriter& writer){
    //We can always put an immediate value everywhere
    writer.stream() << "pushl " << m_operand->getValue() << std::endl;
}
