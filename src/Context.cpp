//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Context.hpp"

using std::map;
using std::string;
using std::endl;
using std::vector;
using std::unordered_set;
using std::unordered_map;

using namespace eddic;

vector<Context*> Context::contexts;



vector<OldContext*> OldContext::contexts;
unsigned int OldContext::currentVariable = 0;

OldContext::~OldContext() {
    map<string, Variable*>::const_iterator it = variables.begin();
    map<string, Variable*>::const_iterator end = variables.end();

    for ( ; it != end; ++it) {
        delete it->second;
    }
}

Variable* OldContext::find(const std::string& variable) {
    map<string, Variable*>::const_iterator it = variables.find(variable);

    if (it == variables.end()) {
        if (m_parent != NULL) {
            return m_parent->find(variable);
        }

        return NULL;
    }

    return it->second;
}

bool OldContext::exists(const std::string& variable) const {
    if (variables.find(variable) != variables.end()) {
        return true;
    }

    if (m_parent != NULL) {
        return m_parent->exists(variable);
    }

    return false;
}

unsigned int OldContext::index(const std::string& variable) const {
    map<string, Variable*>::const_iterator it = variables.find(variable);

    if (it == variables.end()) {
        if (m_parent != NULL) {
            return m_parent->index(variable);
        }

        return -1;
    }

    return it->second->index();
}

Variable* OldContext::create(const std::string& variable, Type type) {
    Variable* v = new Variable(variable, type, currentVariable++);

    variables[variable] = v;

    return v;
}

void OldContext::write(AssemblyFileWriter& writer) {
    map<string, Variable*>::const_iterator it = variables.begin();
    map<string, Variable*>::const_iterator end = variables.end();

    for ( ; it != end; ++it) {
        if (it->second->type() == INT) {
            writer.stream() << ".comm VI" << it->second->index() << ",4,4" << endl;
        } else if (it->second->type() == STRING) {
            writer.stream() << ".comm VS" << it->second->index() << ",8,4" << endl;
        }
    }
}

void OldContext::writeAll(AssemblyFileWriter& writer) {
    for (vector<OldContext*>::const_iterator it = contexts.begin(); it != contexts.end(); ++it) {
        (*it)->write(writer);
    }
}

void OldContext::cleanup(){
    for (vector<OldContext*>::const_iterator it = contexts.begin(); it != contexts.end(); ++it) {
        delete *it;
    }
}

Context::~Context() {
    StoredVariables::const_iterator it = m_stored.begin();
    StoredVariables::const_iterator end = m_stored.end();

    for ( ; it != end; ++it) {
        delete it->second;
    }
}

Variable* Context::addVariable(const std::string& variable, Type type){
    Variable* v = new Variable(variable, type);

    m_visibles.insert(variable);

    storeVariable(variable, v);

    return v;
}

void Context::storeVariable(const std::string& name, Variable* variable){
    m_stored[name] = variable;
}

bool Context::exists(const std::string& variable) const {
    bool found = m_visibles.find(variable) != m_visibles.end();

    if(!found){
        if(m_parent){
            return m_parent->exists(variable);
        }
    }

    return true;
}

Variable* Context::getVariable(const std::string& variable) const {
    bool found = m_visibles.find(variable) != m_visibles.end();

    if(!found){
        if(m_parent){
            return getVariable(variable);
        }
    }

    return (*m_stored.find(variable)).second;
}

void Context::cleanup(){
    for (vector<Context*>::const_iterator it = contexts.begin(); it != contexts.end(); ++it) {
        delete *it;
    }
}

void BlockContext::write(AssemblyFileWriter& writer){
    //Nothing to be done    
}

void FunctionContext::write(AssemblyFileWriter& writer){
    //TODO    
}

void GlobalContext::write(AssemblyFileWriter& writer){
    StoredVariables::const_iterator it = m_stored.begin();
    StoredVariables::const_iterator end = m_stored.end();

    for ( ; it != end; ++it) {
        if (it->second->type() == INT) {
            writer.stream() << ".comm VI" << it->second->index() << ",4,4" << endl;
        } else if (it->second->type() == STRING) {
            writer.stream() << ".comm VS" << it->second->index() << ",8,4" << endl;
        }
    }
}

void BlockContext::addVariable(const std::string variable, Type type){
    Variable* v = new Variable(variable, type);

    m_visibles.insert(variable);

    m_functionContext->storeVariable(variable, v);
}
