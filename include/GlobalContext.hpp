//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef GLOBAL_CONTEXT_H
#define GLOBAL_CONTEXT_H

#include "Context.hpp"

namespace eddic {

/*!
 * \class GlobalContext
 * \brief The symbol table for the whole source. 
 *
 * There is always only one instance of this class in the application. This symbol table is responsible
 * of storing all the global variables. 
 */
class GlobalContext : public Context {
    public:
        GlobalContext();
        
        Variables getVariables();
        
        std::shared_ptr<Variable> addVariable(const std::string& a, std::shared_ptr<const Type> type);
        std::shared_ptr<Variable> addVariable(const std::string& a, std::shared_ptr<const Type> type, ast::Value& value);
};

} //end of eddic

#endif
