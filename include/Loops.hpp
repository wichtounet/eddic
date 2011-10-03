//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LOOPS_H
#define LOOPS_H

#include <memory>

#include "Branches.hpp"

namespace eddic {

class While : public ParseNode {
    private:
        std::shared_ptr<Condition> m_condition;

    public:
        While(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, std::shared_ptr<Condition> condition) : ParseNode(context, token), m_condition(condition) {}

        virtual void write(AssemblyFileWriter& writer);
        virtual void checkVariables();
        virtual void checkStrings(StringPool& pool);
        virtual void optimize();

        std::shared_ptr<Condition> condition() {
            return m_condition;
        }
};

class For : public ParseNode {
    private: 
        std::shared_ptr<ParseNode> m_start;
        std::shared_ptr<ParseNode> m_iter;
        std::shared_ptr<Condition> m_condition;

    public:
        For(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, std::shared_ptr<ParseNode> start, std::shared_ptr<Condition> condition, std::shared_ptr<ParseNode> iter) : ParseNode(context, token), m_start(start), m_iter(iter), m_condition(condition) {}

        virtual void write(AssemblyFileWriter& writer);
        virtual void checkVariables();
        virtual void checkStrings(StringPool& pool);
        virtual void optimize();
};

class Foreach : public ParseNode {
    private: 
        std::shared_ptr<Value> m_from;
        std::shared_ptr<Value> m_to;

    public:
        Foreach(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, std::shared_ptr<Value> from, std::shared_ptr<Value> to) : ParseNode(context, token), m_from(from), m_to(to) {}

        virtual void write(AssemblyFileWriter& writer);
        virtual void checkVariables();
        virtual void checkStrings(StringPool& pool);
        virtual void optimize();
};

} //end of eddic

#endif
