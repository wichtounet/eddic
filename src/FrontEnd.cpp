//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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
