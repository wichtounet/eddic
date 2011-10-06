//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "VariableOperation.hpp"

#include "Value.hpp"
#include "Variable.hpp"

using namespace eddic;

void VariableOperation::checkStrings(StringPool& pool) {
    value->checkStrings(pool);
}

void VariableOperation::write(AssemblyFileWriter& writer) {
    value->write(writer);

    m_var->popFromStack(writer);
}
