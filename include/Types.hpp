//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TYPES_H
#define TYPES_H

#include <string>

namespace eddic {

enum class BaseType : unsigned int {
    STRING,
    INT,
    BOOL,
    FLOAT,
    VOID 
};

#define BASETYPE_COUNT 5

/*!
 * \struct Type
 * \brief A type descriptor.
 * Can describe any type in an EDDI source file. 
 */
class Type {
    private:
        BaseType type;
        bool array;
        bool const_;
        unsigned int m_size;
    
    public:
        Type(BaseType type, bool array, unsigned int size, bool const_);

        BaseType base() const;
        bool isArray() const;
        bool isConst() const;
        unsigned int size() const;

        friend bool operator==(const Type& lhs, const Type& rhs);
        friend bool operator!=(const Type& lhs, const Type& rhs);
};
        
bool operator==(const Type& lhs, const Type& rhs);
bool operator!=(const Type& lhs, const Type& rhs);

Type newType(const std::string& type);

Type newSimpleType(BaseType baseType, bool const_ = false);
Type newSimpleType(const std::string& baseType, bool const_ = false);

Type newArrayType(BaseType baseType, int size = 0);
Type newArrayType(const std::string& baseType, int size = 0);

int size(BaseType type);
int size(Type type);

bool isType(const std::string& type);

} //end of eddic

#endif
