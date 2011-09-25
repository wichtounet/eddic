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

#include <unordered_map>
#include <unordered_set>

#include "Types.hpp"

#include "AssemblyFileWriter.hpp"

namespace eddic {

enum PositionType {
    STACK, 
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
        int m_index;
        Position m_position;

    public:
        Variable(const std::string& name, Type type, Position position) : m_name(name), m_type(type), m_position(position) {}

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
};

typedef std::unordered_map<int, Variable*> StoredVariables;
typedef std::unordered_map<std::string, int> VisibleVariables;

//TODO Improve the way to manage memory of context
//TODO Rename to Context when finished the implementation
class Context {
    private:
        Context* m_parent;
        
        static std::vector<Context*> contexts;

    protected:
        StoredVariables m_stored;
        VisibleVariables m_visibles;
        
        static int currentVariable;

    public:
        Context(Context* parent) : m_parent(parent) {
            contexts.push_back(this);
        }
        virtual ~Context();
        
        virtual Variable* addVariable(const std::string& a, Type type) = 0;
        virtual bool exists(const std::string& a) const;
        virtual Variable* getVariable(const std::string& variable) const;
        virtual Variable* getVariable(int index) const;
        
        virtual void write(AssemblyFileWriter&){
            //Nothing by default    
        }

        virtual void release(AssemblyFileWriter&){
            //Nothing by default
        }
        
        void storeVariable(int index, Variable* variable);

        Context* parent() const  {
            return m_parent;
        }
        
        static void cleanup();
};

class GlobalContext : public Context {
    public:
        GlobalContext() : Context(NULL) {}
        
        void write(AssemblyFileWriter& writer);
        
        Variable* addVariable(const std::string& a, Type type);
};

class FunctionContext : public Context {
    private:
        int currentPosition;

    public:
        FunctionContext(Context* parent) : Context(parent), currentPosition(4) {}
        
        void write(AssemblyFileWriter& writer);
        void release(AssemblyFileWriter& writer);
        
        Variable* addVariable(const std::string& a, Type type);
        Variable* newVariable(const std::string& a, Type type);
};

class BlockContext : public Context {
    private:
        FunctionContext* m_functionContext;

    public:
        BlockContext(Context* parent, FunctionContext* functionContext) : Context(parent), m_functionContext(functionContext){} 
        
        Variable* addVariable(const std::string& a, Type type);
};

} //end of eddic

#endif
