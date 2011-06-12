//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TIMER_H
#define TIMER_H

#include <iostream>
#include <ctime>
#include <cstdlib>
#include <sys/time.h>

using namespace std;

class Timer {
	public:
	Timer();
	double elapsed();
	private:
	timeval startTime;
	timeval endTime;
};

#endif
