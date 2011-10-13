//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MODULO_H
#define MODULO_H

#include <memory>

#include "BinaryOperator.hpp"

namespace eddic {

class Modulo : public BinaryOperator {
    public:
        Modulo(std::shared_ptr<Context> context, const Tok token, std::shared_ptr<Value> lhs, std::shared_ptr<Value> rhs);

        void write(AssemblyFileWriter& writer);

        int compute(int left, int right);
};

} //end of eddic

#endif
