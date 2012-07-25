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
#include "Struct.hpp"

namespace eddic {

typedef std::unordered_map<std::string, std::shared_ptr<Function>> FunctionMap;
typedef std::unordered_map<std::string, std::shared_ptr<Struct>> StructMap;

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
        StructMap structs;

        void addPrintFunction(const std::string& function, std::shared_ptr<const Type> parameterType);
        void defineStandardFunctions();
        
        bool is_recursively_nested(const std::string& struct_, unsigned int left);

    public:
        SymbolTable();
        
        SymbolTable(const SymbolTable& rhs) = delete;
        SymbolTable& operator=(const SymbolTable& rhs) = delete;

        /*!
         * Reset the symbol table. 
         */
        void reset();

        /*!
         * Add the given function to the symbol table. 
         * \param function The function to add to the symbol table. 
         */
        void addFunction(std::shared_ptr<Function> function);
        
        /*!
         * Returns the function with the given name. 
         * \param function The function to search for. 
         * \return A pointer to the function with the given name. 
         */
        std::shared_ptr<Function> getFunction(const std::string& function);
        
        /*!
         * Indicates if a function with the given name exists. 
         * \param function The function to search for. 
         * \return true if a function with the given name exists, otherwise false. 
         */
        bool exists(const std::string& function);

        /*!
         * Add the given structure to the symbol table. 
         * \param struct_ The structure to add to the symbol table. 
         */
        void add_struct(std::shared_ptr<Struct> struct_);
        
        /*!
         * Returns the structure with the given name. 
         * \param struct_ The structure to search for. 
         * \return A pointer to the structure with the given name. 
         */
        std::shared_ptr<Struct> get_struct(const std::string& struct_);
        
        /*!
         * Indicates if a structure with the given name exists. 
         * \param struct_ The structure to search for. 
         * \return true if a structure with the given name exists, otherwise false. 
         */
        bool struct_exists(const std::string& struct_);
        
        int member_offset(std::shared_ptr<Struct> struct_, const std::string& member);
        int size_of_struct(const std::string& struct_);
        bool is_recursively_nested(const std::string& struct_);

        /*!
         * Returns an iterator to the first function. 
         * \return An iterator to the first function. 
         */
        FunctionMap::const_iterator begin();
        
        /*!
         * Returns an iterator past the last function. 
         * \return An iterator past the last function. 
         */
        FunctionMap::const_iterator end();

        /*!
         * Add a reference to the function with the given name. 
         * \param function The function to add a reference to. 
         */
        void addReference(const std::string& function);

        /*!
         * Remove a reference to the function with the given name. 
         * \param function The function to remove a reference from. 
         */
        void removeReference(const std::string& function);

        /*!
         * Get the reference counter of the given function. 
         * \param function The function to add a reference to. 
         * \return The reference counter of the given function. 
         */
        int referenceCount(const std::string& function);
};

/*!
 * The global symbol table. 
 */
extern SymbolTable symbols;

} //end of eddic

#endif
