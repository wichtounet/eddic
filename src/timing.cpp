//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
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

void timing_system::display(){
    std::cout << "Timings" << std::endl;

    typedef std::pair<std::string, double> timer;
    std::vector<timer> timers;
    for(auto& timing : timings){
        timers.emplace_back(timing.first, timing.second);
    }

    std::sort(timers.begin(), timers.end(), [](const timer& lhs, const timer& rhs){ return lhs.second > rhs.second; });

    for(auto& timing : timers){
        std::cout << "    " << timing.first << ":" << timing.second << "ms" << std::endl;
    }
}

void timing_system::register_timing(std::string name, double time){
    timings[name] += time;
}
