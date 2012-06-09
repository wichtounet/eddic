//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <algorithm>
#include <functional>

#include "assert.hpp"
#include "Context.hpp"
#include "Utils.hpp"
#include "Type.hpp"
#include "Types.hpp"

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
    ASSERT_PATH_NOT_TAKEN("Not implemented");
}

std::shared_ptr<Variable> Context::newFloatTemporary(){
    ASSERT_PATH_NOT_TAKEN("Not implemented");
}

std::shared_ptr<Variable> Context::new_temporary(std::shared_ptr<Type> type){
    ASSERT(type->is_standard_type(), "Temporary can only represent standard types"); 

    if(type == INT || type == BOOL){
        return newTemporary();
    } else if(type == FLOAT){
        return newFloatTemporary();
    } else {
        ASSERT_PATH_NOT_TAKEN("Temporary can only represent int, bool and float");
    }
}

std::shared_ptr<Variable> Context::operator[](const std::string& variable) const {
    return getVariable(variable);
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
