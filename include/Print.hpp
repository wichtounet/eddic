//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef PRINT_H
#define PRINT_H

#include <string>

#include "ParseNode.hpp"
#include "Types.hpp"

namespace eddic {

class Value;

class Print : public ParseNode {
    protected:
        Type m_type;
        std::shared_ptr<Value> value;

    public:
        Print(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, std::shared_ptr<Value> v);

        void checkStrings(StringPool& pool);
        void checkVariables();
        void writeIL(IntermediateProgram& program);
};

} //end of eddic

#endif
