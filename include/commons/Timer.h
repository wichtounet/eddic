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
