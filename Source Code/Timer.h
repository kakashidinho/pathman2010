#ifndef _TIMER_
#define _TIMER_

//calculate time with very high precision
#include "windows.h"

class StopWatch {
private:
    LARGE_INTEGER start;//first check point's value
    LARGE_INTEGER stop;//second check point's value
	double LIToSecs( LARGE_INTEGER * L) ; //large integer to seconds
public:
	void Start();//start stopwatch ,set 1st check point
	void Stop();//stop stopwatch , set 2nd check point
	LARGE_INTEGER GetStart() {return start;};//get 1st check point 's value
	LARGE_INTEGER GetStop() {return stop;};//get 2nd check point 's value
	double GetElapsedTime() ;//get elapsed time (in seconds) between 1st and 2nd check points	
	double GetElapsedTime(LARGE_INTEGER& start , LARGE_INTEGER& stop) ;//get elapsed time (in seconds) between <start> and <stop> check points
};


#endif