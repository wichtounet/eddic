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

int Context::currentVariable = 0;

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

void FunctionContext::write(AssemblyFileWriter& writer){
    int s = 0;
    for(auto it : m_stored){
        s += size(it.second->type());
    }

    if(s > 0){
        writer.stream() << "subl $" << s << " , %esp" << std::endl;
    }
}

void FunctionContext::release(AssemblyFileWriter& writer){
    int s = 0;
    for(auto it : m_stored){
        s += size(it.second->type());
    }

    if(s > 0){
        writer.stream() << "addl $" << s << " , %esp" << std::endl;
    }
}

void GlobalContext::write(AssemblyFileWriter& writer){
    for(auto it : m_stored){
        if (it.second->type() == INT) {
            writer.stream() << ".comm VI" << it.second->position().name() << ",4,4" << endl;
        } else if (it.second->type() == STRING) {
            writer.stream() << ".comm VS" << it.second->position().name() << ",8,4" << endl;
        }
    }
}

std::shared_ptr<Variable> GlobalContext::addVariable(const std::string& variable, Type type){
    Position position(GLOBAL, variable);

    std::shared_ptr<Variable> v(new Variable(variable, type, position));

    m_visibles[variable] = currentVariable;

    storeVariable(currentVariable, v);
    
    currentVariable++;

    return v;
}

std::shared_ptr<Variable> FunctionContext::newParameter(const std::string& variable, Type type){
    Position position(PARAMETER, currentParameter);
    
    currentParameter += size(type);

    return std::shared_ptr<Variable>(new Variable(variable, type, position));
}

std::shared_ptr<Variable> FunctionContext::newVariable(const std::string& variable, Type type){
    Position position(STACK, currentPosition);
    
    currentPosition += size(type);

    return std::shared_ptr<Variable>(new Variable(variable, type, position));
}

std::shared_ptr<Variable> FunctionContext::addVariable(const std::string& variable, Type type){
    std::shared_ptr<Variable> v = newVariable(variable, type);

    m_visibles[variable] = currentVariable;

    storeVariable(currentVariable, v);
    
    currentVariable++;

    return v;
}

std::shared_ptr<Variable> FunctionContext::addParameter(const std::string& parameter, Type type){
   std::shared_ptr<Variable> v = newParameter(parameter, type);

   m_visibles[parameter] = currentVariable;

   storeVariable(currentVariable, v);

   currentVariable++;

   return v;
}

std::shared_ptr<Variable> BlockContext::addVariable(const std::string& variable, Type type){
    std::shared_ptr<Variable> v = m_functionContext->newVariable(variable, type);

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
        default:
            throw CompilerException("Variable of invalid type");
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
        default:
            throw CompilerException("Variable of invalid type");
    }
}
