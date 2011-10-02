//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef CONTEXT_H
#define CONTEXT_H

#include <string>
#include <map>
#include <vector>
#include <memory>

#include <unordered_map>
#include <unordered_set>

#include "Types.hpp"

#include "AssemblyFileWriter.hpp"

namespace eddic {

class Value;

enum PositionType {
    STACK, 
    PARAMETER,
    GLOBAL
};

class Position {
    private:
        const PositionType m_type;
        const int m_offset;
        const std::string m_name;

    public:
        Position(PositionType type, int offset) : m_type(type), m_offset(offset), m_name("") {}
        Position(PositionType type, std::string name) : m_type(type), m_offset(0), m_name(name) {}

        bool isStack(){
            return m_type == STACK;
        }

        bool isParameter(){
            return m_type == PARAMETER;
        }

        bool isGlobal(){
            return m_type == GLOBAL;
        }

        int offset(){
            return m_offset;
        }

        std::string name(){
            return m_name;
        }
};

class Variable {
    private:
        const std::string m_name;
        const Type m_type;
        Position m_position;
        std::shared_ptr<Value> m_value;

    public:
        Variable(const std::string& name, Type type, Position position) : m_name(name), m_type(type), m_position(position) {}
        Variable(const std::string& name, Type type, Position position, std::shared_ptr<Value> value) : m_name(name), m_type(type), m_position(position), m_value(value) {}

        void moveToRegister(AssemblyFileWriter& writer, std::string reg);
        void moveToRegister(AssemblyFileWriter& writer, std::string reg1, std::string reg2);

        void moveFromRegister(AssemblyFileWriter& writer, std::string reg);
        void moveFromRegister(AssemblyFileWriter& writer, std::string reg1, std::string reg2);

        void pushToStack(AssemblyFileWriter& writer);
        void popFromStack(AssemblyFileWriter& writer);

        std::string name() const  {
            return m_name;
        }
        
        Type type() const {
            return m_type;
        }
        
        Position position() const {
            return m_position;
        }
        
        std::shared_ptr<Value> value() const {
            return m_value;
        }
};

typedef std::unordered_map<int, std::shared_ptr<Variable>> StoredVariables;
typedef std::unordered_map<std::string, int> VisibleVariables;

class Context {
    private:
        std::shared_ptr<Context> m_parent;
        
    protected:
        StoredVariables m_stored;
        VisibleVariables m_visibles;
        
        static int currentVariable;

    public:
        Context(std::shared_ptr<Context> parent) : m_parent(parent) {}
        
        virtual std::shared_ptr<Variable> addVariable(const std::string& a, Type type) = 0;
        virtual bool exists(const std::string& a) const;
        virtual std::shared_ptr<Variable> getVariable(const std::string& variable) const;
        virtual std::shared_ptr<Variable> getVariable(int index) const;
        
        virtual void write(AssemblyFileWriter&){
            //Nothing by default    
        }

        virtual void release(AssemblyFileWriter&){
            //Nothing by default
        }
        
        void storeVariable(int index, std::shared_ptr<Variable> variable);

        std::shared_ptr<Context> parent() const  {
            return m_parent;
        }
};

class GlobalContext : public Context {
    public:
        GlobalContext() : Context(NULL) {}
        
        void write(AssemblyFileWriter& writer);
        
        std::shared_ptr<Variable> addVariable(const std::string& a, Type type);
        std::shared_ptr<Variable> addVariable(const std::string& a, Type type, std::shared_ptr<Value> value);
};

class FunctionContext : public Context {
    private:
        int currentPosition;
        int currentParameter;

    public:
        FunctionContext(std::shared_ptr<Context> parent) : Context(parent), currentPosition(4), currentParameter(8) {}
        
        void write(AssemblyFileWriter& writer);
        void release(AssemblyFileWriter& writer);
        
        std::shared_ptr<Variable> addVariable(const std::string& a, Type type);
        std::shared_ptr<Variable> addParameter(const std::string& a, Type type);
        std::shared_ptr<Variable> newVariable(const std::string& a, Type type);
        std::shared_ptr<Variable> newParameter(const std::string& a, Type type);
};

class BlockContext : public Context {
    private:
        std::shared_ptr<FunctionContext> m_functionContext;

    public:
        BlockContext(std::shared_ptr<Context> parent, std::shared_ptr<FunctionContext> functionContext) : Context(parent), m_functionContext(functionContext){} 
        
        std::shared_ptr<Variable> addVariable(const std::string& a, Type type);
};

} //end of eddic

#endif
