//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef BASE_TYPE_H
#define BASE_TYPE_H

namespace eddic {

enum class BaseType : unsigned int {
    STRING,
    INT,
    CHAR,
    BOOL,
    FLOAT,
    VOID 
};

#define BASETYPE_COUNT 6

} //end of eddic

#endif
