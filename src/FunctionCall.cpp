//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <algorithm>

#include "FunctionCall.hpp"

#include "AssemblyFileWriter.hpp"
#include "Value.hpp"
#include "Program.hpp"

#include "mangling.hpp"

using namespace eddic;

using std::endl;

FunctionCall::FunctionCall(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, const std::string& function) : ParseNode(context, token), m_function(function) {}

void FunctionCall::addValue(std::shared_ptr<Value> value){
    m_values.push_back(value);
    addLast(value);
}

void FunctionCall::checkFunctions(Program& program){
    m_function_mangled = mangle(m_function, m_values);

    if(!program.exists(m_function_mangled)){
        throw CompilerException("The function \"" + m_function + "()\" does not exists", token());
    }
}

void FunctionCall::write(AssemblyFileWriter& writer){
    for_each(m_values.rbegin(), m_values.rend(), [&](std::shared_ptr<Value> v){ v->write(writer); });

    writer.stream() << "call " << m_function_mangled << endl;
}
