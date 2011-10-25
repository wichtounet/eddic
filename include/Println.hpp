//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef PRINTLN_H
#define PRINTLN_H

#include "Print.hpp"

namespace eddic {

class Println : public Print {
    public:
        Println(std::shared_ptr<Context> context, const Tok token, std::shared_ptr<Value> v);
        
        void writeIL(IntermediateProgram& program);
};

} //end of eddic

#endif
