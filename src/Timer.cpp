//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <cstdlib>

#include "Timer.hpp"

Timer::Timer(){
	gettimeofday(&startTime, NULL);
}

double Timer::elapsed(){
	gettimeofday(&endTime, NULL);
	
	double elapsedTime = (endTime.tv_sec - startTime.tv_sec) * 1000.0;
	elapsedTime += (endTime.tv_usec - startTime.tv_usec) / 1000.0;

	return elapsedTime;
}
