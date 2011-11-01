//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_NODE_H
#define AST_NODE_H

#include <memory>

namespace eddic {

class Context;

struct Node {
    std::shared_ptr<Context> context;
};

} //end of eddic

#endif
