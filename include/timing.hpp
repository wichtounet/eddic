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

        void register_timing(std::string name, std::size_t time);

    private:
        std::unordered_map<std::string, std::size_t> timings;
        std::shared_ptr<Configuration> configuration;
};

class timing_timer {
    public:
        timing_timer(timing_system& system);
        ~timing_timer();

    private:
        timing_system& system;
        StopWatch timer;
};

} //end of eddic

#endif
