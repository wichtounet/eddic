//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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
