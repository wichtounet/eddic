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

using namespace eddic;

Context::Context(std::shared_ptr<Context> parent) : m_parent(parent) {}

std::shared_ptr<Context> Context::parent() const  {
    return m_parent;
}

bool Context::exists(const std::string& variable) const {
    bool found = variables.find(variable) != variables.end();

    auto parent = m_parent;

    while(!found){
        if(parent){
            found = parent->variables.find(variable) != parent->variables.end();
            parent = parent->m_parent;
        } else {
            return false;
        }
    }

    return true;
}

std::shared_ptr<Variable> Context::newTemporary(){
    //By default this method is not implemented for a context
    assert(false);
}

std::shared_ptr<Variable> Context::getVariable(const std::string& variable) const {
    auto iter = variables.find(variable);
    auto end = variables.end();

    auto parent = m_parent;

    while(iter == end){
        iter = parent->variables.find(variable);
        end = parent->variables.end();
        parent = parent->m_parent;
    }
    
    return iter->second;
}

void Context::removeVariable(const std::string& variable){
    auto iter = variables.find(variable);
    auto end = variables.end();

    auto parent = m_parent;
    
    while(iter == end){
        iter = parent->variables.find(variable);
        end = parent->variables.end();
        parent = parent->m_parent;
    }

    variables.erase(iter);
}
 
Context::Variables::const_iterator Context::begin() const {
    return variables.cbegin();
}

Context::Variables::const_iterator Context::end() const {
    return variables.cend();
}
