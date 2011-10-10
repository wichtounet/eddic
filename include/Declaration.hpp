//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef DECLARATION_H
#define DECLARATION_H

#include "VariableOperation.hpp"
#include "Types.hpp"

namespace eddic {

class Declaration : public VariableOperation {
    private:
        Type m_type;

    public:
        Declaration(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, Type type, const std::string& variable, std::shared_ptr<Value> v) : VariableOperation(context, token, variable, v) { m_type = type;  };

        void checkVariables();
};

} //end of eddic

#endif
