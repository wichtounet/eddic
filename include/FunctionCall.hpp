//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef FUNCTION_CALL_H
#define FUNCTION_CALL_H

#include <vector>
#include <string>

#include <memory>

#include "ParseNode.hpp"
#include "Nodes.hpp"

namespace eddic {

class FunctionCall : public ParseNode {
    private:
        std::string m_function;
        std::string m_function_mangled;
        std::vector<std::shared_ptr<Value>> m_values;

    public:
        FunctionCall(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, const std::string& function) : ParseNode(context, token), m_function(function) {}

        void write(AssemblyFileWriter& writer);
        void checkFunctions(Program& program);

        void addValue(std::shared_ptr<Value> value){
            m_values.push_back(value);
            addLast(value);
        }
};

} //end of eddic

#endif
