//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ASSIGNMENT_H
#define ASSIGNMENT_H

#include "VariableOperation.hpp"

namespace eddic {

class Assignment : public VariableOperation {
    public:
        Assignment(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, const std::string& variable, std::shared_ptr<Value> v) : VariableOperation(context, token, variable, v) {};

        void checkVariables();
};

} //end of eddic

#endif
