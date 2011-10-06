//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VARIABLE_OPERATION_H
#define VARIABLE_OPERATION_H

#include <string>

#include "ParseNode.hpp"

namespace eddic {

class Value;
class Variable;

class VariableOperation : public ParseNode {
    protected:
        std::string m_variable;
        std::shared_ptr<Variable> m_var;
        std::shared_ptr<Value> value;
   
    public:
        VariableOperation(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, const std::string& variable, std::shared_ptr<Value> v) : ParseNode(context, token), m_variable(variable), value(v) {};
        
        void checkStrings(StringPool& pool);
        void write(AssemblyFileWriter& writer);
};

} //end of eddic

#endif
