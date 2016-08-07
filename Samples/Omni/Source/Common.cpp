/*
 * Common.cpp
 *
 *  Created on: Nov 27, 2013
 *      Author: cfyz
 */

#include "Common.hpp"

#if defined(_WIN32)
#include <windows.h>
uint64_t GetTime()
{
	return timeGetTime();
}
#endif
#if defined(__linux) || defined(__APPLE__)
#include <unistd.h>
#include <sys/time.h>
uint64_t GetTime()
{
	timeval t;
	gettimeofday(&t, nullptr);
	return t.tv_sec*1000 + t.tv_usec/1000;
}
#endif
