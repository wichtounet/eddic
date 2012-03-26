//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "StopWatch.hpp"

using namespace eddic;

StopWatch::StopWatch() {
    startTime = Clock::now();
}

double StopWatch::elapsed() {
    Clock::time_point endTime = Clock::now();
    milliseconds ms = std::chrono::duration_cast<milliseconds>(endTime - startTime);
    
    return ms.count();
}
