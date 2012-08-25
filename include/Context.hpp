//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef CONTEXT_H
#define CONTEXT_H

#include <string>
#include <memory>

#include <unordered_map>

#include "ast/Value.hpp"

namespace eddic {

class Type;
class Variable;
class IntermediateProgram;
class GlobalContext;

/*!
 * \class Context
 * \brief A symbol table.
 *
 * The symbol table keeps track of each variables (local and global), parameters and temporaries. 
 * The symbol tables are linked to their parent so that the search can be done within all the visibles symbols
 * by going up onto the symbol tables parents.  
 */
class Context {
    private:
        std::shared_ptr<Context> m_parent;
        std::shared_ptr<GlobalContext> global_context;

    protected:
        typedef std::unordered_map<std::string, std::shared_ptr<Variable>> Variables;

        Variables variables;

    public:
        /*!
         * Construct a new Context. 
         * \param parent The parent Context of this new Context. 
         */
        explicit Context(std::shared_ptr<Context> parent);
        explicit Context(std::shared_ptr<Context> parent, std::shared_ptr<GlobalContext> global_context);
        
        Context(const Context& rhs) = delete;
        Context& operator=(const Context& rhs) = delete;

        /*!
         * Add a new variable to this context. 
         * \param name The name of the variable. 
         * \param type The type of the variable. 
         * \return The created variable. 
         */
        virtual std::shared_ptr<Variable> addVariable(const std::string& name, std::shared_ptr<const Type> type) = 0;
        
        /*!
         * Add a new global variable to this context. 
         * \param name The name of the variable. 
         * \param type The type of the variable. 
         * \param value The value of the global variable. 
         * \return The created variable. 
         */
        virtual std::shared_ptr<Variable> addVariable(const std::string& name, std::shared_ptr<const Type> type, ast::Value& value) = 0;
        
        /*!
         * Remove the given variable from this context.  
         * \param variable The name of the variable to remove. 
         */
        virtual void removeVariable(const std::string& variable);

        /*!
         * \brief Create a new temporary of the given type in this context. 
         * This function will fails if the given type is not standard. 
         * \param type The type of variable to create. 
         * \return The created temporary variable. 
         */
        virtual std::shared_ptr<Variable> new_temporary(std::shared_ptr<const Type> type);

        /*!
         * Indicates if the given variable exists in this context. 
         * \param The name of the searched variable. 
         * \return true if the given variable exists, false otherwise. 
         */
        bool exists(const std::string& name) const;

        /*!
         * Returns the variable with the given name. 
         * \param The name of the searched variable. 
         * \return The variable with the given name.  
         */
        std::shared_ptr<Variable> getVariable(const std::string& name) const;
        
        /*!
         * Returns the variable with the given name. 
         * \param The name of the searched variable. 
         * \return The variable with the given name.  
         */
        std::shared_ptr<Variable> operator[](const std::string& name) const;

        /*!
         * Returns an iterator to the first variable. 
         * \return A const iterator pointing to the first variable. 
         */
        Variables::const_iterator begin() const;
        
        /*!
         * Returns an iterator past the last variable.  
         * \return A const iterator pointing past the last variable. 
         */
        Variables::const_iterator end() const;

        /*!
         * Returns the parent of this context. 
         * \return A pointer to the parent context. If the context is global, this methods returns nullptr. 
         */
        std::shared_ptr<Context> parent() const;
        
        std::shared_ptr<GlobalContext> global() const;
};

} //end of eddic

#endif
