//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef FOR_H
#define FOR_H

#include <memory>

#include "ParseNode.hpp"

namespace eddic {

class Condition;

class For : public ParseNode {
    private: 
        std::shared_ptr<ParseNode> m_start;
        std::shared_ptr<ParseNode> m_iter;
        std::shared_ptr<Condition> m_condition;

    public:
        For(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, std::shared_ptr<ParseNode> start, std::shared_ptr<Condition> condition, std::shared_ptr<ParseNode> iter);

        virtual void write(AssemblyFileWriter& writer);
        virtual void checkVariables();
        virtual void checkStrings(StringPool& pool);
        virtual void optimize();
};

} //end of eddic

#endif
