//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BACK_END_H
#define BACK_END_H

#include <memory>
#include <string>

#include "StringPool.hpp"
#include "tac/Program.hpp"

namespace eddic {

class BackEnd {
    public:
        virtual void generate(std::shared_ptr<tac::Program> tacProgram) = 0;

        void set_string_pool(std::shared_ptr<StringPool> pool);
        std::shared_ptr<StringPool> get_string_pool();

    protected:
        std::shared_ptr<StringPool> pool;
};

}

#endif
