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

//TODO Improve encapsulation and constness of these structures

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

        const ParameterType& parameter(std::size_t i) const;
        std::vector<ParameterType>& parameters();
        const std::vector<ParameterType>& parameters() const;

        bool operator==(const Function& rhs) const;
        
        const std::shared_ptr<const Type> returnType;
        const std::string name;
        const std::string mangledName;
        
        std::shared_ptr<FunctionContext> context;

        std::shared_ptr<const Type> struct_type = nullptr;

        int references;
        bool standard = false;
    
    private:
        std::vector<ParameterType> m_parameters;
};

} //end of eddic

#endif
