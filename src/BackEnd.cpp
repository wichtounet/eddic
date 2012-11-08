//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "BackEnd.hpp"

using namespace eddic;
        
void BackEnd::set_string_pool(std::shared_ptr<StringPool> pool){
    this->pool = pool;
}

std::shared_ptr<StringPool> BackEnd::get_string_pool(){
    return pool;
}
        
void BackEnd::set_configuration(std::shared_ptr<Configuration> configuration){
    this->configuration = configuration;
}

std::shared_ptr<Configuration> BackEnd::get_configuration(){
    return configuration;
}
