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
#include <memory>

#include <unordered_map>

#include "Types.hpp"

#include "ParseNode.hpp"

namespace eddic {

class Function;
class Variable;

class Program : public ParseNode {
    private:
        std::unordered_map<std::string, std::shared_ptr<Function>> functions;

    public:
        Program(std::shared_ptr<Context> context) : ParseNode(context) {}
        
        void write(AssemblyFileWriter& writer);
        void addFunction(std::shared_ptr<Function> function);
        bool exists(std::string function);
};

class Value : public ParseNode {
    protected:
        Type m_type;

    public:
        Value(std::shared_ptr<Context> context) : ParseNode(context) {}
        Value(std::shared_ptr<Context> context, const std::shared_ptr<Token> token) : ParseNode(context, token) {}

        Type type() const {
            return m_type;
        };

        virtual bool isConstant();
        virtual std::string getStringValue();
        virtual int getIntValue();
};

class Methods : public ParseNode {
    public:
        Methods(std::shared_ptr<Context> context) : ParseNode(context) {}

        void write(AssemblyFileWriter& writer);
};

class VariableOperation : public ParseNode {
    protected:
        std::string m_variable;
        std::shared_ptr<Variable> m_var;
        std::shared_ptr<Value> value;
   
    public:
        VariableOperation(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, const std::string& variable, std::shared_ptr<Value> v) : ParseNode(context, token), m_variable(variable), value(v) {};
        
        void checkStrings(StringPool& pool);
        void write(AssemblyFileWriter& writer);
};

class Declaration : public VariableOperation {
    private:
        Type m_type;

    public:
        Declaration(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, Type type, const std::string& variable, std::shared_ptr<Value> v) : VariableOperation(context, token, variable, v) { m_type = type;  };

        void checkVariables();
};

class Assignment : public VariableOperation {
    public:
        Assignment(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, const std::string& variable, std::shared_ptr<Value> v) : VariableOperation(context, token, variable, v) {};

        void checkVariables();
};

class Print : public ParseNode {
    protected:
        Type m_type;
        std::shared_ptr<Value> value;

    public:
        Print(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, std::shared_ptr<Value> v) : ParseNode(context, token), value(v) {};

        void checkStrings(StringPool& pool);
        void checkVariables();
        void write(AssemblyFileWriter& writer);
};

class Println : public Print {
    public:
        Println(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, std::shared_ptr<Value> v) : Print(context, token, v) {}
        void write(AssemblyFileWriter& writer);
};

class Swap : public ParseNode {
    private:
        std::string m_lhs;
        std::string m_rhs;
        std::shared_ptr<Variable> m_lhs_var;
        std::shared_ptr<Variable> m_rhs_var;
        Type m_type;

    public:
        Swap(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, const std::string& lhs, const std::string& rhs) : ParseNode(context, token), m_lhs(lhs), m_rhs(rhs) {};

        void checkVariables();
        void write(AssemblyFileWriter& writer);
};

class Litteral : public Value {
    private:
        std::string m_litteral;
        std::string m_label;
    public:
        Litteral(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, const std::string& litteral) : Value(context, token), m_litteral(litteral) {
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
        Integer(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, int value) : Value(context, token), m_value(value) {
            m_type = INT;
        };
        void write(AssemblyFileWriter& writer);
        bool isConstant();
        int getIntValue();
};

class VariableValue : public Value {
    private:
        std::string m_variable;
        std::shared_ptr<Variable> m_var;
    
    public:
        VariableValue(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, const std::string& variable) : Value(context, token), m_variable(variable) {};
        void checkVariables();
        void write(AssemblyFileWriter& writer);
        bool isConstant();
};

} //end of eddic

#endif
