//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "FrontEnd.hpp"

using namespace eddic;
        
void FrontEnd::set_string_pool(std::shared_ptr<StringPool> pool){
    this->pool = pool;
}
