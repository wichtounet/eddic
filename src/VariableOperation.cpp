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

VariableOperation::VariableOperation(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, const std::string& variable, std::shared_ptr<Value> v) : ParseNode(context, token), m_variable(variable), value(v) {}

void VariableOperation::checkStrings(StringPool& pool) {
    value->checkStrings(pool);
}

void VariableOperation::write(AssemblyFileWriter& writer) {
    value->write(writer);

    m_var->popFromStack(writer);
}

void VariableOperation::writeIL(IntermediateProgram& program){
   value->assignTo(m_var, program); 
}
