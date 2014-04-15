//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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
