//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TYPES_H
#define TYPES_H

namespace eddic {

enum Type {
    STRING = 0,
    INT = 1,
    VOID = 2
};

int size(Type type);

bool isType(std::string type);

Type stringToType(std::string type);

} //end of eddic

#endif
