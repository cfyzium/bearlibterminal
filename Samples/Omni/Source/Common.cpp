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
#endif
#if defined(__linux)
#include <unistd.h>
void delay(int ms)
{
	usleep(ms*1000);
}
#endif
