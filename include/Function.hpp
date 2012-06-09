//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef FUNCTION_H
#define FUNCTION_H

#include <memory>
#include <string>
#include <vector>

namespace eddic {

class FunctionContext;
class Type;

/*!
 * \class ParameterType
 * \brief A parameter for a function.  
 */
struct ParameterType {
    std::string name;
    std::shared_ptr<Type> paramType;

    ParameterType(const std::string& n, std::shared_ptr<Type> t);
};

/*!
 * \class Function
 * \brief A function entry in the function table. 
 */
struct Function {
    std::shared_ptr<Type> returnType;
    std::string name;
    std::string mangledName;
    std::vector<ParameterType> parameters;
    std::shared_ptr<FunctionContext> context;
    int references;

    Function(std::shared_ptr<Type> ret, const std::string& n);

    std::shared_ptr<Type> getParameterType(const std::string& name);
    unsigned int getParameterPositionByType(const std::string& name);
};

} //end of eddic

#endif
