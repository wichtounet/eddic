//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <algorithm>
#include <functional>

#include "Context.hpp"
#include "Utils.hpp"
#include "Value.hpp"

using std::map;
using std::string;
using std::endl;
using std::vector;
using std::unordered_set;
using std::unordered_map;

using namespace eddic;

int Context::currentVariable = 0;

Context::Context(std::shared_ptr<Context> parent) : m_parent(parent) {}

void Context::write(AssemblyFileWriter&){
    //Nothing by default    
}

void Context::release(AssemblyFileWriter&){
    //Nothing by default
}

std::shared_ptr<Context> Context::parent() const  {
    return m_parent;
}

void Context::storeVariable(int index, std::shared_ptr<Variable> variable){
    m_stored[index] = variable;
}

bool Context::exists(const std::string& variable) const {
    bool found = m_visibles.find(variable) != m_visibles.end();

    if(!found){
        if(m_parent){
            return m_parent->exists(variable);
        }
    }

    return found;
}

std::shared_ptr<Variable> Context::getVariable(const std::string& variable) const {
    auto iter = m_visibles.find(variable);

    if(iter == m_visibles.end()){
        return m_parent->getVariable(variable);
    }
    
    return getVariable(iter->second);
}

std::shared_ptr<Variable> Context::getVariable(int index) const {
    auto iter = m_stored.find(index);

    if(iter == m_stored.end()){
        return m_parent->getVariable(index);
    }

    return iter->second;
}
