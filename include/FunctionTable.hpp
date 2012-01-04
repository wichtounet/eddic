//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef FUNCTION_TABLE_H
#define FUNCTION_TABLE_H

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "Types.hpp"

namespace eddic {

/*!
 * \class ParameterType
 * \brief A parameter for a function.  
 */
struct ParameterType {
    std::string name;
    Type paramType;

    ParameterType(const std::string& n, Type t) : name(n), paramType(t) {}
};

/*!
 * \class Function
 * \brief A function entry in the function table. 
 */
struct Function {
    Type returnType;
    std::string name;
    std::string mangledName;
    std::vector<ParameterType> parameters;
    int references;

    Function(Type ret, const std::string& n) : returnType(ret), name(n), references(0) {}
};

/*!
 * \class FunctionTable
 * \brief The global function table. 
 * 
 * This class is responsible for managing all the functions of the current program. It's also responsible
 * of managing the reference count for the functions.  
 */
class FunctionTable {
    private:
        std::unordered_map<std::string, std::shared_ptr<Function>> functions;

    public:
        FunctionTable();
        FunctionTable(const FunctionTable& rhs) = delete;

        void addFunction(std::shared_ptr<Function> function);
        std::shared_ptr<Function> getFunction(const std::string& function);
        bool exists(const std::string& function);

        void addReference(const std::string& function);
        int referenceCount(const std::string& function);
};

} //end of eddic

#endif
