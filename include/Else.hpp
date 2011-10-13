//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ELSE_H
#define ELSE_H

#include "ParseNode.hpp"

namespace eddic {

class Else : public ParseNode {
    public:
        Else(std::shared_ptr<Context> context, Tok token) : ParseNode(context, token) {}
};

} //end of eddic

#endif
