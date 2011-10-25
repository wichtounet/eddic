//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <algorithm>
#include <functional>

#include "GlobalContext.hpp"
#include "CompilerException.hpp"
#include "Variable.hpp"
#include "Utils.hpp"
#include "Value.hpp"

#include "il/IntermediateProgram.hpp"

using std::map;
using std::string;
using std::endl;
using std::vector;

using namespace eddic;

void GlobalContext::writeIL(IntermediateProgram& program){
    for(auto it : m_stored){
        if (it.second->type() == Type::INT) {
            program.addInstruction(program.factory().createGlobalIntVariable(it.second->position().name(), it.second->value()->getIntValue()));
        } else if (it.second->type() == Type::STRING) {
            program.addInstruction(program.factory().createGlobalStringVariable(it.second->position().name(), it.second->value()->getStringLabel(), it.second->value()->getStringSize()));
        }
    }
}

std::shared_ptr<Variable> GlobalContext::addVariable(const std::string&, Type){
    //TODO throw CompilerException("A global variable must have a value");
    throw "Test";
}

std::shared_ptr<Variable> GlobalContext::addVariable(const std::string& variable, Type type, std::shared_ptr<Value> value){
    Position position(GLOBAL, variable);

    std::shared_ptr<Variable> v(new Variable(variable, type, position, value));

    m_visibles[variable] = currentVariable;

    storeVariable(currentVariable, v);
    
    currentVariable++;

    return v;
}
