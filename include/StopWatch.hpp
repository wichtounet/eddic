//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef STOP_WATCH_H
#define STOP_WATCH_H

#include <chrono>

namespace eddic {

typedef std::chrono::high_resolution_clock Clock;

/*!
 * \class StopWatch
 * \brief Simple stopwatch to keep track of elapsed time between two points. 
 * 
 * The stopwatch starts automatically when instantiated. This stopwatch use Boost Chrono to keep tracks of time.    
 */
class StopWatch {
    public:
        StopWatch();
        double elapsed();
        double micro_elapsed();
	
    private:
        Clock::time_point startTime;
};

} //end of eddic

#endif
