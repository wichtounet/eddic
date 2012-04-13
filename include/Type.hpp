//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TYPE_H
#define TYPE_H

#include <string>
#include <boost/optional.hpp>

namespace eddic {

#define BASETYPE_COUNT 5

enum class BaseType : unsigned int {
    STRING,
    INT,
    BOOL,
    FLOAT,
    VOID 
};

/*!
 * \struct Type
 * \brief A type descriptor.
 * Can describe any type in an EDDI source file. 
 */
class Type {
    private:
        bool array;
        bool const_;
        bool custom;
        
        boost::optional<BaseType> baseType;
        boost::optional<std::string> m_type;
        boost::optional<unsigned int> m_size;
    
    public:
        Type(BaseType type, bool array, unsigned int size, bool const_);
        Type(const std::string& type);

        BaseType base() const;
        bool isArray() const;
        bool isConst() const;
        bool is_custom_type() const;
        bool is_standard_type() const;

        unsigned int size() const;
        std::string type() const;

        friend bool operator==(const Type& lhs, const Type& rhs);
        friend bool operator!=(const Type& lhs, const Type& rhs);

        friend bool operator==(const Type& lhs, const BaseType& rhs);
        friend bool operator!=(const Type& lhs, const BaseType& rhs);
};
        
bool operator==(const Type& lhs, const Type& rhs);
bool operator!=(const Type& lhs, const Type& rhs);

bool operator==(const Type& lhs, const BaseType& rhs);
bool operator!=(const Type& lhs, const BaseType& rhs);

} //end of eddic

#endif
