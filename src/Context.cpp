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

using std::string;
using std::endl;
using std::vector;
using std::unordered_map;

using namespace eddic;

Context::Context(std::shared_ptr<Context> parent) : m_parent(parent) {}

void Context::writeIL(IntermediateProgram&){
    //Nothing by default    
}

std::shared_ptr<Context> Context::parent() const  {
    return m_parent;
}

int Context::size(){
    return 0;
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

std::shared_ptr<Variable> Context::addVariable(const std::string&, Type, ast::Value&){
    //By default this method is not implemented for a context
    assert(false);
}

std::shared_ptr<Variable> Context::getVariable(const std::string& variable) const {
    auto iter = variables.find(variable);

    if(iter == variables.end()){
        return m_parent->getVariable(variable);
    }
    
    return iter->second;
}

void Context::removeVariable(const std::string& variable){
    auto iter = variables.find(variable);

    if(iter == variables.end()){
        return m_parent->removeVariable(variable);
    }

    variables.erase(iter);
}
 
Context::Variables::const_iterator Context::begin(){
    return variables.cbegin();
}

Context::Variables::const_iterator Context::end(){
    return variables.cend();
}
