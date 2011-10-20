//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef METHODS_H
#define METHODS_H

#include "ParseNode.hpp"

namespace eddic {

class Methods : public ParseNode {
    public:
        Methods(std::shared_ptr<Context> context) : ParseNode(context) {}

        void writeIL(IntermediateProgram& program);
};

} //end of eddic

#endif
