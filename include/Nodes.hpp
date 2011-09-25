//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef NODES_H
#define NODES_H

#include <string>
#include <map>

#include <unordered_map>

#include "Types.hpp"

#include "ParseNode.hpp"

namespace eddic {

class Function;
class Variable;

class Program : public ParseNode {
    private:
        std::unordered_map<std::string, Function*> functions;

    public:
        Program(Context* context) : ParseNode(context) {}
        ~Program();
        
        void write(AssemblyFileWriter& writer);
        void addFunction(Function* function);
        bool exists(std::string function);
};

class Value : public ParseNode {
    protected:
        Type m_type;

    public:
        Value(Context* context) : ParseNode(context) {}

        Type type() const {
            return m_type;
        };

        virtual bool isConstant();
        virtual std::string getStringValue();
        virtual int getIntValue();
};

class Methods : public ParseNode {
    public:
        Methods(Context* context) : ParseNode(context) {}

        void write(AssemblyFileWriter& writer);
};

class VariableOperation : public ParseNode {
    protected:
        std::string m_variable;
        Variable* m_var;
        Value* value;
   
    public:
        VariableOperation(Context* context, const std::string& variable, Value* v) : ParseNode(context), m_variable(variable), value(v) {};
        ~VariableOperation() {
            delete value;
        }
        
        void checkStrings(StringPool& pool);
        void write(AssemblyFileWriter& writer);
};

class Declaration : public VariableOperation {
    private:
        Type m_type;

    public:
        Declaration(Context* context, Type type, const std::string& variable, Value* v) : VariableOperation(context, variable, v) { m_type = type; };

        void checkVariables();
};

class Assignment : public VariableOperation {
    public:
        Assignment(Context* context, const std::string& variable, Value* v) : VariableOperation(context, variable, v) {};

        void checkVariables();
};

class Print : public ParseNode {
    protected:
        Type m_type;
        Value* value;

    public:
        Print(Context* context, Value* v) : ParseNode(context), value(v) {};
        virtual ~Print() {
            delete value;
        }

        void checkStrings(StringPool& pool);
        void checkVariables();
        void write(AssemblyFileWriter& writer);
};

class Println : public Print {
    public:
        Println(Context* context, Value*v) : Print(context, v) {}
        void write(AssemblyFileWriter& writer);
};

class Swap : public ParseNode {
    private:
        std::string m_lhs;
        std::string m_rhs;
        Variable* m_lhs_var;
        Variable* m_rhs_var;
        Type m_type;

    public:
        Swap(Context* context, const std::string& lhs, const std::string& rhs) : ParseNode(context), m_lhs(lhs), m_rhs(rhs) {};

        void checkVariables();
        void write(AssemblyFileWriter& writer);
};

class Litteral : public Value {
    private:
        std::string m_litteral;
        std::string m_label;
    public:
        Litteral(Context* context, const std::string& litteral) : Value(context), m_litteral(litteral) {
            m_type = STRING;
        };
        void checkStrings(StringPool& pool);
        void write(AssemblyFileWriter& writer);
        bool isConstant();
        std::string getStringValue();
};

class Integer : public Value {
    private:
        int m_value;
    public:
        Integer(Context* context, int value) : Value(context), m_value(value) {
            m_type = INT;
        };
        void write(AssemblyFileWriter& writer);
        bool isConstant();
        int getIntValue();
};

class VariableValue : public Value {
    private:
        std::string m_variable;
        Variable* m_var;
    
    public:
        VariableValue(Context* context, const std::string& variable) : Value(context), m_variable(variable) {};
        void checkVariables();
        void write(AssemblyFileWriter& writer);
        bool isConstant();
};

} //end of eddic

#endif
