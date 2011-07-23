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

using namespace eddic;

vector<Context*> Context::contexts;
unsigned int Context::currentVariable = 0;

Context::~Context() {
    map<string, Variable*>::const_iterator it = variables.begin();
    map<string, Variable*>::const_iterator end = variables.end();

    for ( ; it != end; ++it) {
        delete it->second;
    }
}

Variable* Context::find(const std::string& variable) {
    map<string, Variable*>::const_iterator it = variables.find(variable);

    if (it == variables.end()) {
        if (m_parent != NULL) {
            return m_parent->find(variable);
        }

        return NULL;
    }

    return it->second;
}

bool Context::exists(const std::string& variable) const {
    if (variables.find(variable) != variables.end()) {
        return true;
    }

    if (m_parent != NULL) {
        return m_parent->exists(variable);
    }

    return false;
}

unsigned int Context::index(const std::string& variable) const {
    map<string, Variable*>::const_iterator it = variables.find(variable);

    if (it == variables.end()) {
        if (m_parent != NULL) {
            return m_parent->index(variable);
        }

        return -1;
    }

    return it->second->index();
}

Variable* Context::create(const std::string& variable, Type type) {
    Variable* v = new Variable(variable, type, currentVariable++);

    variables[variable] = v;

    return v;
}

void Context::write(AssemblyFileWriter& writer) {
    map<string, Variable*>::const_iterator it = variables.begin();
    map<string, Variable*>::const_iterator end = variables.end();

    for ( ; it != end; ++it) {
        if (it->second->type() == INT) {
            writer.stream() << "VI" << it->second->index() << ":" << endl;
            writer.stream() << ".long 0" << endl;
        } else if (it->second->type() == STRING) {
            writer.stream() << "VS" << it->second->index() << ":" << endl;
            writer.stream() << ".long 0" << endl;
            writer.stream() << "VS" << it->second->index() << "_l:" << endl;
            writer.stream() << ".long 0" << endl;
        }
    }
}

void Context::writeAll(AssemblyFileWriter& writer) {
    for (vector<Context*>::const_iterator it = contexts.begin(); it != contexts.end(); ++it) {
        (*it)->write(writer);
    }
}
