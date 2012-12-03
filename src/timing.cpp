//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "timing.hpp"

using namespace eddic;

timing_timer::timing_timer(timing_system& system) : system(system) {
    //Nothing
}

timing_timer::~timing_timer(){
    //TODO
}

timing_system::timing_system(std::shared_ptr<Configuration> configuration) : configuration(configuration) {
    //Nothing
}

timing_system::~timing_system(){
    //TODO    
}

void timing_system::register_timing(std::string name, std::size_t time){
    timings[name] += time;
}
