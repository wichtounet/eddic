//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef PARAMETER_H
#define PARAMETER_H

#include <memory>
#include <string>

namespace eddic {

class Type;

/*!
 * \struct Parameter
 * \brief A parameter for a function.  
 */
class Parameter {
    public:
        Parameter(const std::string& name, std::shared_ptr<const Type> type);
        
        //Parameter cannot be copied
        Parameter(const Parameter& rhs) = delete;
        Parameter& operator=(const Parameter& rhs) = delete;
        
        //Parameter can be moved
        Parameter(const Parameter&& rhs);
        Parameter& operator=(const Parameter&& rhs);

        const std::string& name() const;
        const std::shared_ptr<const Type>& type() const;

    private:
        std::string _name;
        std::shared_ptr<const Type> _type;
};

} //end of eddic

#endif
