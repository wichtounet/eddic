//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "FrontEnd.hpp"

using namespace eddic;
        
void FrontEnd::set_string_pool(std::shared_ptr<StringPool> pool){
    this->pool = pool;
}

std::shared_ptr<StringPool> FrontEnd::get_string_pool(){
    return pool;
}
        
void FrontEnd::set_configuration(std::shared_ptr<Configuration> configuration){
    this->configuration = configuration;
}

std::shared_ptr<Configuration> FrontEnd::get_configuration(){
    return configuration;
}
