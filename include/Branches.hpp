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
#include <memory>

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
        std::shared_ptr<Value> m_lhs;
        std::shared_ptr<Value> m_rhs;

    public:
        Condition(BooleanCondition condition) : m_condition(condition), m_lhs(NULL), m_rhs(NULL) {}
        Condition(BooleanCondition condition, std::shared_ptr<Value> lhs, std::shared_ptr<Value> rhs) : m_condition(condition), m_lhs(lhs), m_rhs(rhs) {}

        std::shared_ptr<Value> lhs() {
            return m_lhs;
        }
        std::shared_ptr<Value> rhs() {
            return m_rhs;
        }
        BooleanCondition condition() const {
            return m_condition;
        }
        bool isOperator() const {
            return m_lhs != NULL || m_rhs != NULL;
        }
};

void writeJumpIfNot(AssemblyFileWriter& writer, std::shared_ptr<Condition> condition, std::string label, int labelIndex);

class Else : public ParseNode {
    public:
        Else(Context* context, Token* token) : ParseNode(context, token) {}
};

class ElseIf : public ParseNode {
    private:
        std::shared_ptr<Condition> m_condition;

    public:
        ElseIf(Context* context, Token* token, std::shared_ptr<Condition> condition) : ParseNode(context, token), m_condition(condition) {}

        virtual void checkVariables();
        virtual void checkStrings(StringPool& pool);
        virtual void optimize();

        std::shared_ptr<Condition> condition() {
            return m_condition;
        }
};

class If : public ParseNode {
    private:
        std::shared_ptr<Condition> m_condition;
        std::shared_ptr<Else> m_elseBlock;
        std::vector<std::shared_ptr<ElseIf>> elseIfs;

    public:
        If(Context* context, const Token* token, std::shared_ptr<Condition> condition) : ParseNode(context, token), m_condition(condition) {}

        virtual void write(AssemblyFileWriter& writer);
        virtual void checkVariables();
        virtual void checkStrings(StringPool& pool);
        virtual void optimize();

        void setElse(std::shared_ptr<Else> elseBlock) {
            m_elseBlock = elseBlock;
        }
        void addElseIf(std::shared_ptr<ElseIf> elseIf) {
            elseIfs.push_back(elseIf);
        }

        std::shared_ptr<Condition> condition() {
            return m_condition;
        }
};

} //end of eddic

#endif
