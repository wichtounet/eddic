//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TIMING_H
#define TIMING_H

#include <memory>

#include "Options.hpp"
#include "StopWatch.hpp"

namespace eddic {

class timing_system {
    public:
        timing_system(std::shared_ptr<Configuration> configuration);
        ~timing_system();

        void register_timing(std::string name, double time);

    private:
        std::unordered_map<std::string, double> timings;
        std::shared_ptr<Configuration> configuration;
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
