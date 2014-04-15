//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef TIMING_H
#define TIMING_H

#include <memory>
#include <unordered_map>

#include "Options.hpp"
#include "StopWatch.hpp"

namespace eddic {

class timing_system {
    public:
        void register_timing(std::string name, double time);
        void display();

    private:
        std::unordered_map<std::string, double> timings;
};

class timing_timer {
    public:
        timing_timer(timing_system& system, const std::string& name);
        ~timing_timer();

    private:
        timing_system& system;
        std::string name;
        StopWatch timer;
};

} //end of eddic

#endif
