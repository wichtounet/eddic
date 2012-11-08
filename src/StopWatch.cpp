//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "StopWatch.hpp"

using namespace eddic;

typedef std::chrono::milliseconds milliseconds;
typedef std::chrono::microseconds microseconds;

StopWatch::StopWatch() {
    startTime = Clock::now();
}

template<typename Precision>
inline double elapsed_time(Clock::time_point& start){
    Clock::time_point endTime = Clock::now();
    Precision ms = std::chrono::duration_cast<Precision>(endTime - start);
    
    return ms.count();
}

double StopWatch::elapsed() {
    return elapsed_time<milliseconds>(startTime);
}

double StopWatch::micro_elapsed() {
    return elapsed_time<microseconds>(startTime);
}
