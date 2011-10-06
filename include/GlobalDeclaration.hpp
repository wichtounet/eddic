//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef GLOBAL_DECLARATION_H
#define GLOBAL_DECLARATION_H

#include <string>

#include "ParseNode.hpp"
#include "Types.hpp"
#include "GlobalContext.hpp"

namespace eddic {

class Variable;
class Value;

class GlobalDeclaration : public ParseNode {
    private:
        std::shared_ptr<GlobalContext> m_globalContext;
        Type m_type;
        std::string m_variable;
        std::shared_ptr<Variable> m_var;
        std::shared_ptr<Value> value;

    public:
        GlobalDeclaration(std::shared_ptr<GlobalContext> context, const std::shared_ptr<Token> token, Type type, const std::string& variable, std::shared_ptr<Value> v) : ParseNode(context, token), m_globalContext(context), m_variable(variable), value(v){ m_type = type;  };

        void checkStrings(StringPool& pool);
        void checkVariables();
};

} //end of eddic

#endif
