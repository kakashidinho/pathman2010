#include "stdafx.h"
#include "../Timer.h"
void StopWatch::Start() {
	QueryPerformanceCounter(&start) ;
}

void StopWatch::Stop() {
	QueryPerformanceCounter(&stop) ;
}

double StopWatch::LIToSecs( LARGE_INTEGER * L) {
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency( &frequency ) ;
	return ((double)L->QuadPart /(double)frequency.QuadPart) ;
}

double StopWatch::GetElapsedTime() {
	LARGE_INTEGER time;
	time.QuadPart = stop.QuadPart - start.QuadPart;
	return LIToSecs( &time) ;
}

double StopWatch::GetElapsedTime(LARGE_INTEGER& start , LARGE_INTEGER& stop) {
	LARGE_INTEGER time;
	time.QuadPart = stop.QuadPart - start.QuadPart;
	return LIToSecs( &time) ;
}