//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BLOCK_CONTEXT_H
#define BLOCK_CONTEXT_H

#include <string>
#include <map>
#include <vector>
#include <memory>

#include "Types.hpp"
#include "Context.hpp"

namespace eddic {

class FunctionContext;
class Variable;

class BlockContext : public Context {
    private:
        std::shared_ptr<FunctionContext> m_functionContext;

    public:
        BlockContext(std::shared_ptr<Context> parent, std::shared_ptr<FunctionContext> functionContext) : Context(parent), m_functionContext(functionContext){} 
        
        std::shared_ptr<Variable> addVariable(const std::string& a, Type type);
};

} //end of eddic

#endif
