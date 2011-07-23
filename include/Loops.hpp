//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LOOPS_H
#define LOOPS_H

#include "Branches.hpp"

namespace eddic {

class While : public ParseNode {
    private:
        Condition* m_condition;

    public:
        While(Context* context, Condition* condition) : ParseNode(context), m_condition(condition) {}

        virtual ~While();

        virtual void write(AssemblyFileWriter& writer);
        virtual void checkVariables();
        virtual void checkStrings(StringPool& pool);
        virtual void optimize();

        Condition* condition() {
            return m_condition;
        }
};

} //end of eddic

#endif
