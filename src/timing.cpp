//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "timing.hpp"

using namespace eddic;

timing_timer::timing_timer(timing_system& system, const std::string& name) : system(system), name(name) {
    //Nothing
}

timing_timer::~timing_timer(){
    system.register_timing(name, timer.elapsed());
}

timing_system::timing_system(std::shared_ptr<Configuration> configuration) : configuration(configuration) {
    //Nothing
}

timing_system::~timing_system(){
    if(configuration->option_defined("time")){
        for(auto& timing : timings){
            std::cout << timing.first << ":" << timing.second << "ms" << std::endl;
        }
    }
}

void timing_system::register_timing(std::string name, double time){
    timings[name] += time;
}
