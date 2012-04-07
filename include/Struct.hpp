//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef STRUCT_H
#define STRUCT_H

#include <memory>
#include <string>
#include <vector>

#include "Types.hpp"

namespace eddic {

/*!
 * \class Member
 * \brief A member of a function.  
 */
struct Member {
    std::string name;
    Type paramType;

    Member(const std::string& n, Type t);
};

/*!
 * \class Function
 * \brief A function entry in the function table. 
 */
struct Struct {
    std::string name;
    std::vector<Member> members;
    int references = 0;
    
    Struct(const std::string& n);
};

} //end of eddic

#endif
