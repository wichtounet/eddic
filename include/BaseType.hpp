//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BASE_TYPE_H
#define BASE_TYPE_H

namespace eddic {

//TODO Verify if this is still necessary
#define BASETYPE_COUNT 5

enum class BaseType : unsigned int {
    STRING,
    INT,
    BOOL,
    FLOAT,
    VOID 
};

} //end of eddic

#endif
