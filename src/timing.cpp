//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>

#include "timing.hpp"

using namespace eddic;

timing_timer::timing_timer(timing_system& system, const std::string& name) : system(system), name(name) {
    //Nothing
}

timing_timer::~timing_timer(){
    system.register_timing(name, timer.elapsed());
}

bool is_aggregate(const std::string& name){
    return name == "whole_optimizations" || name == "all_optimizations";
}

void timing_system::display(){
    std::cout << "Timers" << std::endl;

    typedef std::pair<std::string, double> timer;
    std::vector<timer> timers;
    for(auto& timing : timings){
        timers.emplace_back(timing.first, timing.second);
    }

    std::sort(timers.begin(), timers.end(), 
            [](const timer& lhs, const timer& rhs){ return lhs.second > rhs.second; });

    double total = 0.0;
    
    for(auto& timing : timers){
        if(!is_aggregate(timing.first)){
            total += timing.second;
        }
    }

    for(auto& timing : timers){
        if(!is_aggregate(timing.first)){
            size_t save_prec = std::cout.precision();
            std::cout << "    " << timing.first << ":" << timing.second << "ms (" << std::setprecision(2) << ((timing.second / total) * 100) << "%)"<< std::endl;
            std::cout.precision(save_prec);
        }
    }
    
    std::cout << "Aggregate Timers" << std::endl;
    std::cout << "    " << "Total:" << total << "ms" << std::endl;
    
    for(auto& timing : timers){
        if(is_aggregate(timing.first)){
            std::cout << "    " << timing.first << ":" << timing.second << "ms" << std::endl;
        }
    }
}

void timing_system::register_timing(std::string name, double time){
    timings[name] += time;
}
