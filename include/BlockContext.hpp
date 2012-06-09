//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BLOCK_CONTEXT_H
#define BLOCK_CONTEXT_H

#include "Context.hpp"

namespace eddic {

class FunctionContext;

/*!
 * \class BlockContext
 * \brief A symbol table for the block level. 
 */
class BlockContext : public Context {
    private:
        std::shared_ptr<FunctionContext> m_functionContext;

    public:
        BlockContext(std::shared_ptr<Context> parent, std::shared_ptr<FunctionContext> functionContext);
        
        std::shared_ptr<Variable> addVariable(const std::string& a, std::shared_ptr<Type> type);
        std::shared_ptr<Variable> addVariable(const std::string& a, std::shared_ptr<Type> type, ast::Value& value);

        std::shared_ptr<Variable> newTemporary();
        std::shared_ptr<Variable> newFloatTemporary();
};

} //end of eddic

#endif
