//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <algorithm>
#include <functional>

#include "GlobalContext.hpp"
#include "Variable.hpp"
#include "Utils.hpp"
#include "Value.hpp"

using std::map;
using std::string;
using std::endl;
using std::vector;

using namespace eddic;

void GlobalContext::write(AssemblyFileWriter& writer){
    for(auto it : m_stored){
        if (it.second->type() == Type::INT) {
            writer.stream() << ".size VI" << it.second->position().name() << ", 4" << endl;
            writer.stream() << "VI" << it.second->position().name() << ":" << endl;
            writer.stream() << ".long " << it.second->value()->getIntValue() << endl;
        } else if (it.second->type() == Type::STRING) {
            writer.stream() << ".size VS" << it.second->position().name() << ", 8" << endl;
            writer.stream() << "VS" << it.second->position().name() << ":" << endl;
            writer.stream() << ".long " << it.second->value()->getStringLabel() << endl;
            writer.stream() << ".long " << it.second->value()->getStringSize() << endl;
        }
    }
}

std::shared_ptr<Variable> GlobalContext::addVariable(const std::string& variable, Type type){
    throw CompilerException("A global variable must have a value");
}

std::shared_ptr<Variable> GlobalContext::addVariable(const std::string& variable, Type type, std::shared_ptr<Value> value){
    Position position(GLOBAL, variable);

    std::shared_ptr<Variable> v(new Variable(variable, type, position, value));

    m_visibles[variable] = currentVariable;

    storeVariable(currentVariable, v);
    
    currentVariable++;

    return v;
}
