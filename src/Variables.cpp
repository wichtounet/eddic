//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Variables.hpp"

using std::map;
using std::string;

using namespace eddic;

Variables::~Variables() {
    map<string, Variable*>::const_iterator it = variables.begin();
    map<string, Variable*>::const_iterator end = variables.end();

    for( ; it != end; ++it) {
        delete it->second;
    }
}

Variable* Variables::find(const std::string& variable) {
    map<string, Variable*>::const_iterator it = variables.find(variable);

    if(it == variables.end()) {
        return NULL;
    }

    return it->second;
}

bool Variables::exists(const std::string& variable) const {
    return variables.find(variable) != variables.end();
}

unsigned int Variables::index(const std::string& variable) const {
    map<string, Variable*>::const_iterator it = variables.find(variable);

    if(it == variables.end()) {
        return -1;
    }

    return it->second->index();
}

Variable* Variables::create(const std::string& variable, Type type) {
    Variable* v = new Variable(variable, type, currentVariable++);

    variables[variable] = v;

    return v;
}

void Variables::write(ByteCodeFileWriter& writer) {
    map<string, Variable*>::const_iterator it = variables.begin();
    map<string, Variable*>::const_iterator end = variables.end();

    for( ; it != end; ++it) {
        if(it->second->type() == INT) {
            writer.stream() << "VI" << it->second->index() << ":" << std::endl;
            writer.stream() << ".long 0" << std::endl;
        } else if(it->second->type() == STRING) {
            writer.stream() << "VS" << it->second->index() << ":" << std::endl;
            writer.stream() << ".long 0" << std::endl;
            writer.stream() << ".long 0" << std::endl;
        }
    }
}

