//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef FUNCTION_CONTEXT_H
#define FUNCTION_CONTEXT_H

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
        int temporary = 1;

        //Refer all variables that are stored, including temporary
        Variables storage;

    public:
        FunctionContext(std::shared_ptr<Context> parent);
        
        int size() const;

        std::shared_ptr<Variable> addVariable(const std::string& a, std::shared_ptr<const Type> type);
        std::shared_ptr<Variable> addVariable(const std::string& a, std::shared_ptr<const Type> type, ast::Value& value);
        std::shared_ptr<Variable> addParameter(const std::string& a, std::shared_ptr<const Type> type);
        std::shared_ptr<Variable> newVariable(const std::string& a, std::shared_ptr<const Type> type);
        std::shared_ptr<Variable> newParameter(const std::string& a, std::shared_ptr<const Type> type);

        std::shared_ptr<Variable> newVariable(std::shared_ptr<Variable> source);
        std::shared_ptr<Variable> newTemporary();
        std::shared_ptr<Variable> newFloatTemporary();
        
        void removeVariable(const std::string& variable) override;

        void storeTemporary(std::shared_ptr<Variable> temp);

        Variables stored_variables();
};

} //end of eddic

#endif
