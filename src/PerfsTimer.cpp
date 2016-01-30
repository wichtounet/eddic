//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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
