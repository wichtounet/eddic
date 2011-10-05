//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef WHILE_H
#define WHILE_H

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

} //end of eddic

#endif
