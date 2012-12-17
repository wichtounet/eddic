//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
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
 * \struct ParameterType
 * \brief A parameter for a function.  
 */
struct ParameterType {
    std::string name;
    std::shared_ptr<const Type> paramType;

    ParameterType(const std::string& n, std::shared_ptr<const Type> t);
};

/*!
 * \class Function
 * \brief A function entry in the function table. 
 */
class Function {
    public:
        Function(std::shared_ptr<const Type> ret, const std::string& name, const std::string& mangled_name);

        Function(const Function& rhs) = delete;
        Function& operator=(const Function& rhs) = delete;

        std::shared_ptr<const Type> getParameterType(const std::string& name) const;
        unsigned int getParameterPositionByType(const std::string& name) const;
        
        std::shared_ptr<const Type> returnType;
        std::string name;
        std::string mangledName;
        std::string struct_;
        std::vector<ParameterType> parameters;
        std::shared_ptr<FunctionContext> context;

        std::shared_ptr<const Type> struct_type = nullptr;

        int references;
        bool standard = false;
};

} //end of eddic

#endif
