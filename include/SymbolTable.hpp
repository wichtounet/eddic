//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <memory>
#include <string>
#include <unordered_map>

#include "Variable.hpp"
#include "Function.hpp"

namespace eddic {

typedef std::unordered_map<std::string, std::shared_ptr<Function>> FunctionMap;

/*!
 * \class SymbolTable
 * \brief The global symbol table. 
 * 
 * This class is responsible for managing all the functions and the structs declarations of the current program. 
 * It's also responsible of managing the reference count for the functions.  
 */
class SymbolTable {
    private:
        FunctionMap functions;

        void addPrintFunction(const std::string& function, BaseType parameterType);
        void defineStandardFunctions();

    public:
        SymbolTable();
        SymbolTable(const SymbolTable& rhs) = delete;

        void addFunction(std::shared_ptr<Function> function);
        std::shared_ptr<Function> getFunction(const std::string& function);
        bool exists(const std::string& function);

        FunctionMap::const_iterator begin();
        FunctionMap::const_iterator end();

        void addReference(const std::string& function);
        int referenceCount(const std::string& function);
};

} //end of eddic

#endif
