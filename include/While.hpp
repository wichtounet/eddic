//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef WHILE_H
#define WHILE_H

#include <memory>

#include "ParseNode.hpp"

namespace eddic {

class Condition;

class While : public ParseNode {
    private:
        std::shared_ptr<Condition> m_condition;

    public:
        While(std::shared_ptr<Context> context, const Tok token, std::shared_ptr<Condition> condition);

        virtual void writeIL(IntermediateProgram& program);
        
        virtual void checkVariables();
        virtual void checkStrings(StringPool& pool);
        virtual void optimize();

        std::shared_ptr<Condition> condition();
};

} //end of eddic

#endif
