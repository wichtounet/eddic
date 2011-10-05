//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef IF_H
#define IF_H

#include <vector>
#include <string>
#include <memory>

#include "ParseNode.hpp"

namespace eddic {

class Condition;
class Else;
class ElseIf;

class If : public ParseNode {
    private:
        std::shared_ptr<Condition> m_condition;
        std::shared_ptr<Else> m_elseBlock;
        std::vector<std::shared_ptr<ElseIf>> elseIfs;

    public:
        If(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, std::shared_ptr<Condition> condition) : ParseNode(context, token), m_condition(condition) {}

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

void writeJumpIfNot(AssemblyFileWriter& writer, std::shared_ptr<Condition> condition, std::string label, int labelIndex);

} //end of eddic

#endif
