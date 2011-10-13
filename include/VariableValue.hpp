//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VARIABLE_VALUE_H
#define VARIABLE_VALUE_H

#include <string>

#include "Value.hpp"

namespace eddic {

class Variable;

class VariableValue : public Value {
    private:
        std::string m_variable;
        std::shared_ptr<Variable> m_var;
    
    public:
        VariableValue(std::shared_ptr<Context> context, const Tok& token, const std::string& variable);
        VariableValue(std::shared_ptr<Context> context, const Tok& token, std::shared_ptr<Variable> var);

        void checkVariables();
        void write(AssemblyFileWriter& writer);
        bool isConstant();
};

} //end of eddic

#endif
