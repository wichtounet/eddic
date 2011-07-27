//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BRANCHES_H
#define BRANCHES_H

#include <vector>
#include <string>

#include "Nodes.hpp"

namespace eddic {

enum BooleanCondition {
    GREATER_OPERATOR,
    LESS_OPERATOR,
    EQUALS_OPERATOR,
    NOT_EQUALS_OPERATOR,
    GREATER_EQUALS_OPERATOR,
    LESS_EQUALS_OPERATOR,
    TRUE_VALUE,
    FALSE_VALUE
};

class Condition {
    private:
        BooleanCondition m_condition;
        Value* m_lhs;
        Value* m_rhs;

    public:
        Condition(BooleanCondition condition) : m_condition(condition), m_lhs(NULL), m_rhs(NULL) {}
        Condition(BooleanCondition condition, Value* lhs, Value* rhs) : m_condition(condition), m_lhs(lhs), m_rhs(rhs) {}
        ~Condition() {
            delete m_lhs;
            delete m_rhs;
        }

        Value* lhs() {
            return m_lhs;
        }
        Value* rhs() {
            return m_rhs;
        }
        BooleanCondition condition() const {
            return m_condition;
        }
        bool isOperator() const {
            return m_lhs != NULL || m_rhs != NULL;
        }
};

void writeJumpIfNot(AssemblyFileWriter& writer, Condition* condition, std::string label, int labelIndex);

class Else : public ParseNode {
    public:
        Else(Context* context) : ParseNode(context) {}
};

class ElseIf : public ParseNode {
    private:
        Condition* m_condition;

    public:
        ElseIf(Context* context, Condition* condition) : ParseNode(context), m_condition(condition) {}

        virtual ~ElseIf() {
            delete m_condition;
        }

        virtual void checkVariables();
        virtual void checkStrings(StringPool& pool);
        virtual void optimize();

        Condition* condition() {
            return m_condition;
        }
};

class If : public ParseNode {
    private:
        Condition* m_condition;
        Else* m_elseBlock;
        std::vector<ElseIf*> elseIfs;

    public:
        If(Context* context, Condition* condition) : ParseNode(context), m_condition(condition), m_elseBlock(NULL) {}

        virtual ~If();

        virtual void write(AssemblyFileWriter& writer);
        virtual void checkVariables();
        virtual void checkStrings(StringPool& pool);
        virtual void optimize();

        void setElse(Else* elseBlock) {
            m_elseBlock = elseBlock;
        }
        void addElseIf(ElseIf* elseIf) {
            elseIfs.push_back(elseIf);
        }

        Condition* condition() {
            return m_condition;
        }
};

class For : public ParseNode {
    private: 
        ParseNode* m_start;
        ParseNode* m_iter;
        Condition* m_condition;

    public:
        For(Context* context, ParseNode* start, ParseNode* iter, Condition* condition) : ParseNode(context), m_start(start), m_iter(iter), m_condition(condition) {}

        virtual ~For();

        virtual void write(AssemblyFileWriter& writer);
        virtual void checkVariables();
        virtual void checkStrings(StringPool& pool);
        virtual void optimize();
};

} //end of eddic

#endif
