//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TIMER_H
#define TIMER_H

#include <boost/chrono.hpp>

namespace eddic {

/*!
 * \class Timer
 * \brief Simple stopwatch to keep track of elapsed time between two points. 
 * 
 * The stopwatch starts automatically when instantiated. This stopwatch use Boost Chrono to keep tracks of time.    
 */
class Timer {
    public:
        Timer();
        double elapsed();
	
    private:
        boost::chrono::system_clock::time_point startTime;
};

} //end of eddic

#endif
