//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef FUNCTION_H
#define FUNCTION_H

#include <memory>
#include <string>
#include <vector>

#include "Parameter.hpp"

namespace eddic {

class FunctionContext;
class Type;

/*!
 * \class Function
 * \brief A function entry in the function table. 
 */
class Function {
    public:
        Function(std::shared_ptr<const Type> ret, std::string name, std::string mangled_name);

        //Function cannot be copied
        Function(const Function& rhs) = delete;
        Function& operator=(const Function& rhs) = delete;
        
        //Function can be moved
        Function(Function&& rhs);
        Function& operator=(Function&& rhs);

        const Parameter& parameter(std::size_t i) const;
        const Parameter& parameter(const std::string& name) const;

        std::vector<Parameter>& parameters();
        const std::vector<Parameter>& parameters() const;

        unsigned int parameter_position_by_type(const std::string& name) const;

        const std::shared_ptr<const Type>& return_type() const;
        const std::string& name() const;
        const std::string& mangled_name() const;

        std::shared_ptr<FunctionContext>& context();
        const std::shared_ptr<FunctionContext>& context() const;

        std::shared_ptr<const Type>& struct_type();

        bool standard() const;
        bool& standard();
    
    private:
        std::shared_ptr<FunctionContext> _context;
        std::shared_ptr<const Type> _struct_type = nullptr;

        std::shared_ptr<const Type> _return_type;
        std::string _name;
        std::string _mangled_name;

        bool _standard = false;

        std::vector<Parameter> _parameters;
};

bool operator==(const Function& lhs, const Function& rhs);
bool operator!=(const Function& lhs, const Function& rhs);

} //end of eddic

#endif
