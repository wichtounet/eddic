//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef FUNCTION_CONTEXT_H
#define FUNCTION_CONTEXT_H

#include <string>
#include <memory>

#include "Types.hpp"

#include "Context.hpp"

namespace eddic {

/*!
 * \class FunctionContext
 * \brief A symbol table for a function.
 */
class FunctionContext : public Context {
    private:
        int currentPosition;
        int currentParameter;
        int m_size;
        int temporary;

    public:
        FunctionContext(std::shared_ptr<Context> parent);
        
        int size() const;

        std::shared_ptr<Variable> addVariable(const std::string& a, Type type);
        std::shared_ptr<Variable> addVariable(const std::string& a, Type type, ast::Value& value);
        std::shared_ptr<Variable> addParameter(const std::string& a, Type type);
        std::shared_ptr<Variable> newVariable(const std::string& a, Type type);
        std::shared_ptr<Variable> newParameter(const std::string& a, Type type);

        std::shared_ptr<Variable> newTemporary();
        std::shared_ptr<Variable> newFloatTemporary();

        void storeTemporary(std::shared_ptr<Variable> temp);
};

} //end of eddic

#endif
