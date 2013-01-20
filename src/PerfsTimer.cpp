//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "PerfsTimer.hpp"
#include "logging.hpp"

using namespace eddic;

PerfsTimer::PerfsTimer(const std::string& n) : name(n) {}
PerfsTimer::PerfsTimer(const std::string& n, bool precise) : name(n), precise(precise) {}

PerfsTimer::~PerfsTimer(){
    if(log::enabled<Trace>()){
        if(precise){
            LOG<Trace>("Perfs") << name << " took " << timer.micro_elapsed() << "us" << log::endl;
        } else {
            LOG<Trace>("Perfs") << name << " took " << timer.elapsed() << "ms" << log::endl;
        }
    }
}
