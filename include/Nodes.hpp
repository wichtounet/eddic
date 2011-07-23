//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef NODES_H
#define NODES_H

#include <string>

#include "Types.hpp"

#include "ParseNode.hpp"

namespace eddic {

class Program : public ParseNode {
	public:
		Program(Context* context) : ParseNode(context) {}
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

class Header : public ParseNode {
    public:
		Header(Context* context) : ParseNode(context) {}

        void write(AssemblyFileWriter& writer);

};

class Exit : public ParseNode {
    public:
		Exit(Context* context) : ParseNode(context) {}

        void write(AssemblyFileWriter& writer);
};

class Methods : public ParseNode {
    public:
		Methods(Context* context) : ParseNode(context) {}

        void write(AssemblyFileWriter& writer);
};

class Declaration : public ParseNode {
    private:
        Type m_type;
        std::string m_variable;
        int m_index;
        Value* value;
		
    public:
        Declaration(Context* context, Type type, const std::string& variable, Value* v) : ParseNode(context), m_type(type), m_variable(variable), value(v) {};
        ~Declaration() {
            delete value;
        }

        void checkStrings(StringPool& pool);
        void checkVariables();
        void write(AssemblyFileWriter& writer);
        int index() const {
            return m_index;
        }
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

class Assignment : public ParseNode {
    private:
        std::string m_variable;
        int m_index;
        Value* value;

    public:
        Assignment(Context* context, const std::string& variable, Value* v) : ParseNode(context), m_variable(variable), value(v) {};
        ~Assignment() {
            delete value;
        }

        void checkStrings(StringPool& pool);
        void checkVariables();
        void write(AssemblyFileWriter& writer);
};

class Swap : public ParseNode {
    private:
        std::string m_lhs;
        std::string m_rhs;
        int m_lhs_index;
        int m_rhs_index;
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
        int m_index;
    public:
        VariableValue(Context* context, const std::string& variable) : Value(context), m_variable(variable) {};
        void checkVariables();
        void write(AssemblyFileWriter& writer);
        bool isConstant();
};

} //end of eddic

#endif
