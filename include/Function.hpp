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

#include "Parameter.hpp"

namespace eddic {

class FunctionContext;
class Type;

//TODO Improve encapsulation and constness of these structures

/*!
 * \class Function
 * \brief A function entry in the function table. 
 */
class Function {
    public:
        Function(std::shared_ptr<const Type> ret, const std::string& name, const std::string& mangled_name);

        //Function cannot be copied
        Function(const Function& rhs) = delete;
        Function& operator=(const Function& rhs) = delete;

        unsigned int getParameterPositionByType(const std::string& name) const;

        const Parameter& parameter(std::size_t i) const;
        const Parameter& parameter(const std::string& name) const;

        std::vector<Parameter>& parameters();
        const std::vector<Parameter>& parameters() const;

        bool operator==(const Function& rhs) const;

        const std::shared_ptr<const Type>& return_type() const;
        const std::string& name();
        const std::string& mangled_name();

        std::shared_ptr<FunctionContext>& context();
        const std::shared_ptr<FunctionContext>& context() const;

        std::shared_ptr<const Type>& struct_type();

        int references() const;
        int& references();

        bool standard() const;
        bool& standard();
    
    private:
        std::shared_ptr<FunctionContext> _context;
        std::shared_ptr<const Type> _struct_type = nullptr;

        std::shared_ptr<const Type> _return_type;
        std::string _name;
        std::string _mangled_name;

        int _references = 0;
        bool _standard = false;

        std::vector<Parameter> _parameters;
};

} //end of eddic

#endif
