/*
* BearLibTerminal
* Copyright (C) 2013-2016 Cfyz
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
* of the Software, and to permit persons to whom the Software is furnished to do
* so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
* IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "Utility.hpp"
#include "Log.hpp"

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__linux)
#include <sys/time.h>
#endif

namespace BearLibTerminal
{
	bool try_parse(const std::wstring& s, bool& out)
	{
		if (s == L"true" || s == L"1")
		{
			out = true;
			return true;
		}
		else if (s == L"false" || s == L"0")
		{
			out = false;
			return true;
		}
		else
		{
			return false;
		}
	}

	bool try_parse(const std::wstring& s, char32_t& out)
	{
		if (s.empty())
			return false;

		if (s.length() > 2 && ((s[0] == L'0' && (s[1] == L'x' || s[1] == L'X')) || (s[0] == L'U' && s[1] == L'+')))
		{
			std::wistringstream stream(s.substr(2));
			stream >> std::hex;
			uint32_t temp;
			stream >> temp;
			out = temp;
			return !(stream.fail() || stream.bad());
		}
		else if (s.length() == 3 && s[0] == L'\'' && s[2] == L'\'')
		{
			out = (uint16_t)s[1];
			return true;
		}
		else
		{
			std::wistringstream stream(s);
			uint32_t temp;
			stream >> temp;
			out = temp;
			return !(stream.fail() || stream.bad());
		}
	}

	bool try_parse(const std::wstring& s, uint64_t& out)
	{
		if (s.empty()) return false; // Hard to find anything less than 32-bit now

		if (s.length() > 2 && s[0] == L'0' && (s[1] == L'x' || s[1] == L'X'))
		{
			std::wistringstream stream(s.substr(2));
			stream >> std::hex;
			stream >> out;
			return !(stream.fail() || stream.bad());
		}
		else
		{
			std::wistringstream stream(s);
			stream >> out;
			return !(stream.fail() || stream.bad());
		}
	}

	bool try_parse(const std::wstring& s, wchar_t& out)
	{
		uint16_t temp;
		if (!try_parse(s, temp)) return false;
		out = (wchar_t)temp;
		return true;
	}

#if defined(_WIN32)
	static float get_timer_factor()
	{
		LARGE_INTEGER f;
		QueryPerformanceFrequency(&f);
		return f.QuadPart / 1E+6f;
	}

	uint64_t gettime()
	{
		static float factor = get_timer_factor();
		LARGE_INTEGER t;
		QueryPerformanceCounter(&t);
		return t.QuadPart / factor;
	}
#elif defined(__linux)
	uint64_t gettime()
	{
		timeval t;
		gettimeofday(&t, nullptr);
		return t.tv_sec*1000000 + t.tv_usec;
	}
#endif
}
