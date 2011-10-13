//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ELSE_IF_H
#define ELSE_IF_H

#include <vector>
#include <string>
#include <memory>

#include "ParseNode.hpp"

namespace eddic {

class Condition;

class ElseIf : public ParseNode {
    private:
        std::shared_ptr<Condition> m_condition;

    public:
        ElseIf(std::shared_ptr<Context> context, const Tok& token, std::shared_ptr<Condition> condition) : ParseNode(context, token), m_condition(condition) {}

        virtual void checkVariables();
        virtual void checkStrings(StringPool& pool);
        virtual void optimize();

        std::shared_ptr<Condition> condition() {
            return m_condition;
        }
};

} //end of eddic

#endif
