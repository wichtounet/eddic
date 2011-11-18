//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef FUNCTION_CONTEXT_H
#define FUNCTION_CONTEXT_H

#include <string>
#include <map>
#include <vector>
#include <memory>

#include <unordered_map>
#include <unordered_set>

#include "Types.hpp"

#include "Context.hpp"

namespace eddic {

class Value;

class FunctionContext : public Context {
    private:
        int currentPosition;
        int currentParameter;
        int m_size;

    public:
        FunctionContext(std::shared_ptr<Context> parent) : Context(parent), currentPosition(4), currentParameter(8) {}
        
        int size();

        std::shared_ptr<Variable> addVariable(const std::string& a, Type type);
        std::shared_ptr<Variable> addParameter(const std::string& a, Type type);
        std::shared_ptr<Variable> newVariable(const std::string& a, Type type);
        std::shared_ptr<Variable> newParameter(const std::string& a, Type type);
};

} //end of eddic

#endif
