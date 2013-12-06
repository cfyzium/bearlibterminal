/*
 * Common.cpp
 *
 *  Created on: Nov 27, 2013
 *      Author: cfyz
 */

#include "Common.hpp"

#if defined(_WIN32)
#include <Windows.h>
void delay(int ms)
{
	Sleep(ms);
}
uint64_t GetTime()
{
	return timeGetTime();
}
#endif
#if defined(__linux)
#include <unistd.h>
#include <sys/time.h>
void delay(int ms)
{
	usleep(ms*1000);
}
uint64_t GetTime()
{
	timeval t;
	gettimeofdat(&t, nullptr);
	return t.tv_sec*1000 + t.tv_usec/1000;
}
#endif
