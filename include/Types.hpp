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
    VOID, 

    COUNT
};

class Type {
    private:
        BaseType type;
        bool array;
        unsigned int m_size;
    
    public:
        Type(BaseType base);
        Type(BaseType base, unsigned int size);
        
        BaseType base() const;
        bool isArray() const;
        unsigned int size() const;

        friend bool operator==(const Type& lhs, const Type& rhs);
        friend bool operator!=(const Type& lhs, const Type& rhs);
};
        
bool operator==(const Type& lhs, const Type& rhs);
bool operator!=(const Type& lhs, const Type& rhs);

int size(BaseType type);
int size(Type type);

bool isType(const std::string& type);

Type stringToType(const std::string& type);

} //end of eddic

#endif
