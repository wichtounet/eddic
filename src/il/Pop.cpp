//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <cassert>
#include <iostream>

#include "il/Pop.hpp"
#include "il/Operand.hpp"

#include "AssemblyFileWriter.hpp"

using namespace eddic;

Pop::Pop(std::shared_ptr<Operand> o) : operand(o) {}

void Pop::write(AssemblyFileWriter& writer) const {
    assert(operand->isRegister());

    writer.stream() << "popl " << operand->getValue() << std::endl;
}
