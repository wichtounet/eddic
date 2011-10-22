//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Timer.hpp"

using namespace eddic;

Timer::Timer() {
    startTime = boost::chrono::system_clock::now();
}

double Timer::elapsed() {
    boost::chrono::duration<double> sec = boost::chrono::system_clock::now() - startTime;
    
    return sec.count();
}
