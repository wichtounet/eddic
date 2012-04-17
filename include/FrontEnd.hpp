//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef FRONT_END_H
#define FRONT_END_H

#include <memory>
#include <string>

#include "StringPool.hpp"
#include "tac/Program.hpp"

namespace eddic {

class FrontEnd {
    public:
        virtual std::shared_ptr<tac::Program> compile(const std::string& file) = 0;   

        void set_string_pool(std::shared_ptr<StringPool> pool);
        std::shared_ptr<StringPool> get_string_pool();

    protected:
        std::shared_ptr<StringPool> pool;
};

}

#endif
