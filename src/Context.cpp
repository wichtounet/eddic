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

using std::map;
using std::string;
using std::endl;
using std::vector;
using std::unordered_set;
using std::unordered_map;

using namespace eddic;

vector<Context*> Context::contexts;
int Context::currentVariable = 0;

Context::~Context() {
    StoredVariables::const_iterator it = m_stored.begin();
    StoredVariables::const_iterator end = m_stored.end();

    for ( ; it != end; ++it) {
        delete it->second;
    }
}

void Context::storeVariable(int index, Variable* variable){
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

Variable* Context::getVariable(const std::string& variable) const {
    VisibleVariables::const_iterator iter = m_visibles.find(variable);

    if(iter == m_visibles.end()){
        return m_parent->getVariable(variable);
    }
    
    return getVariable(iter->second);
}

Variable* Context::getVariable(int index) const {
    StoredVariables::const_iterator iter = m_stored.find(index);

    if(iter == m_stored.end()){
        return m_parent->getVariable(index);
    }

    return iter->second;
}

void Context::cleanup(){
    for_each(contexts.begin(), contexts.end(), deleter());
}

void FunctionContext::write(AssemblyFileWriter& writer){
    StoredVariables::const_iterator it = m_stored.begin();
    StoredVariables::const_iterator end = m_stored.end();

    int s = 0;
    
    for ( ; it != end; ++it) {
        s += size(it->second->type());
    }

    if(s > 0){
        writer.stream() << "subl $" << s << " , %esp" << std::endl;
    }
}

void FunctionContext::release(AssemblyFileWriter& writer){
    StoredVariables::const_iterator it = m_stored.begin();
    StoredVariables::const_iterator end = m_stored.end();

    int s = 0;
    
    for ( ; it != end; ++it) {
        s += size(it->second->type());
    }

    if(s > 0){
        writer.stream() << "addl $" << s << " , %esp" << std::endl;
    }
}

void GlobalContext::write(AssemblyFileWriter& writer){
    StoredVariables::const_iterator it = m_stored.begin();
    StoredVariables::const_iterator end = m_stored.end();

    for ( ; it != end; ++it) {
        if (it->second->type() == INT) {
            writer.stream() << ".comm VI" << it->second->position().name() << ",4,4" << endl;
        } else if (it->second->type() == STRING) {
            writer.stream() << ".comm VS" << it->second->position().name() << ",8,4" << endl;
        }
    }
}

Variable* GlobalContext::addVariable(const std::string& variable, Type type){
    Position position(GLOBAL, variable);

    Variable* v = new Variable(variable, type, position);

    m_visibles[variable] = currentVariable;

    storeVariable(currentVariable, v);
    
    currentVariable++;

    return v;
}

Variable* FunctionContext::newParameter(const std::string& variable, Type type){
    Position position(PARAMETER, currentParameter);
    
    currentParameter += size(type);

    return new Variable(variable, type, position);
}

Variable* FunctionContext::newVariable(const std::string& variable, Type type){
    Position position(STACK, currentPosition);
    
    currentPosition += size(type);

    return new Variable(variable, type, position);
}

Variable* FunctionContext::addVariable(const std::string& variable, Type type){
    Variable* v = newVariable(variable, type);

    m_visibles[variable] = currentVariable;

    storeVariable(currentVariable, v);
    
    currentVariable++;

    return v;
}

Variable* FunctionContext::addParameter(const std::string& parameter, Type type){
   Variable* v = newParameter(parameter, type);

   m_visibles[parameter] = currentVariable;

   storeVariable(currentVariable, v);

   currentVariable++;

   return v;
}

Variable* BlockContext::addVariable(const std::string& variable, Type type){
    Variable* v = m_functionContext->newVariable(variable, type);

    m_visibles[variable] = currentVariable;

    m_functionContext->storeVariable(currentVariable, v);
    
    currentVariable++;

    return v;
}

void Variable::moveToRegister(AssemblyFileWriter& writer, std::string reg){
    if(m_type == INT){ 
        if(m_position.isStack()){
            writer.stream() << "movl -" << m_position.offset() << "(%ebp), " << reg << endl;
        } else if(m_position.isParameter()){
            writer.stream() << "movl " << m_position.offset() << "(%ebp), " << reg << endl;
        } else if(m_position.isGlobal()){
            writer.stream() << "movl VI" << m_position.name() << ", " << reg << endl;
        }
   } else if (m_type == STRING){
       //TODO Should never be called 
   }
}

void Variable::moveToRegister(AssemblyFileWriter& writer, std::string reg1, std::string reg2){
    if(m_type == INT){ 
       //TODO Should never be called 
   } else if (m_type == STRING){
       if(m_position.isStack()){
           writer.stream() << "movl -" << m_position.offset() << "(%ebp), " << reg1 << endl;
           writer.stream() << "movl -" << (m_position.offset() + 4) << "(%ebp), " << reg2 << endl;
       } else if(m_position.isParameter()){
           writer.stream() << "movl " << m_position.offset() << "(%ebp), " << reg1 << endl;
           writer.stream() << "movl " << (m_position.offset() + 4) << "(%ebp), " << reg2 << endl;
       } else {
           writer.stream() << "movl VS" << m_position.name() << ", " << reg1 << endl;
           writer.stream() << "movl VS" << m_position.name() << "+4, " << reg2 << endl;
       }
   }
}

void Variable::moveFromRegister(AssemblyFileWriter& writer, std::string reg){
    if(m_type == INT){ 
        if(m_position.isStack()){
            writer.stream() << "movl " << reg << ", -" << m_position.offset() << "(%ebp)" << endl;
        } else if(m_position.isParameter()){
            writer.stream() << "movl " << reg << ", " << m_position.offset() << "(%ebp)" << endl;
        } else if(m_position.isGlobal()){
            writer.stream() << "movl " << reg << ", VI" << m_position.name()  << endl;
        }
   } else if (m_type == STRING){
       //TODO Should never be called 
   }
}

void Variable::moveFromRegister(AssemblyFileWriter& writer, std::string reg1, std::string reg2){
    if(m_type == INT){ 
       //TODO Should never be called 
   } else if (m_type == STRING){
       if(m_position.isStack()){
           writer.stream() << "movl " << reg1 << ", -" << m_position.offset() << "(%ebp)" << endl;
           writer.stream() << "movl " << reg2 << ", -" << (m_position.offset() + 4) << "(%ebp)" << endl;
       } else if(m_position.isParameter()){
           writer.stream() << "movl " << reg1 << ", " << m_position.offset() << "(%ebp)" << endl;
           writer.stream() << "movl " << reg2 << ", " << (m_position.offset() + 4) << "(%ebp)" << endl;
       } else {
           writer.stream() << "movl " << reg1 << ", VS" << m_position.name() << endl;
           writer.stream() << "movl " << reg2 << ", VS" << m_position.name() << "+4" << endl;
       }
   }
}

void Variable::pushToStack(AssemblyFileWriter& writer){
    switch (m_type) {
        case INT:
            if(m_position.isStack()){
                writer.stream() << "pushl -" << m_position.offset() << "(%ebp)" << std::endl;
            } else if(m_position.isParameter()){
                writer.stream() << "pushl " << m_position.offset() << "(%ebp)" << std::endl;
            } else if(m_position.isGlobal()){
                writer.stream() << "pushl VI" << m_position.name() << std::endl;
            }

            break;
        case STRING:
            if(m_position.isStack()){
                writer.stream() << "pushl -" << (m_position.offset() + 4) << "(%ebp)" << std::endl;
                writer.stream() << "pushl -" << m_position.offset() << "(%ebp)" << std::endl;
            } else if(m_position.isParameter()){
                writer.stream() << "pushl " << (m_position.offset() + 4) << "(%ebp)" << std::endl;
                writer.stream() << "pushl " << m_position.offset() << "(%ebp)" << std::endl;
            } else if(m_position.isGlobal()){
                writer.stream() << "pushl VS" << m_index << endl;
                writer.stream() << "pushl VS" << m_index << "+4" << std::endl;
            }

            break;
    }
}

void Variable::popFromStack(AssemblyFileWriter& writer){
    switch (m_type) {
        case INT:
            writer.stream() << "movl (%esp), %eax" << endl;
            writer.stream() << "addl $4, %esp" << endl;

            moveFromRegister(writer, "%eax");

            break;
        case STRING:
            writer.stream() << "movl (%esp), %eax" << endl;
            writer.stream() << "movl 4(%esp), %ebx" << endl;
            writer.stream() << "addl $8, %esp" << endl;

            moveFromRegister(writer, "%eax", "%ebx");
            
            break;
    }
}
