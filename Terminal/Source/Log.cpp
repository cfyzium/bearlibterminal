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

#include "Log.hpp"
#include "Encoding.hpp"
#include "Utility.hpp"
#include "Platform.hpp"
#include "Config.hpp"
#include <fstream>
#include <time.h>
#include <chrono>

namespace BearLibTerminal
{
	static std::string FormatTime()
	{
		auto now = std::chrono::system_clock::now();
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
		time_t time = std::chrono::system_clock::to_time_t(now);
		struct tm tm = {0};
#if defined(_MSC_VER)
		localtime_s(&tm, &time); // MSVC
#else
		localtime_r(&time, &tm); // SUSv2
#endif

		const size_t buffer_size = 13;
		char buffer[buffer_size] = {0};
		snprintf
		(
			buffer,
			buffer_size,
			"%02d:%02d:%02d.%03d", // hh:mm:ss.ttt
			tm.tm_hour,
			tm.tm_min,
			tm.tm_sec,
			(int)(ms%1000)
		);

		return buffer;
	}

	Log& Log::Instance()
	{
		static Log instance;
		return instance;
	}

	Log::Log()
	{
		EnsureStandardOutput();
		Reset();
	}

	void Log::Reset()
	{
		GetEnvironmentVariable(L"BEARLIB_LOGFILE", filename);
		try_parse(GetEnvironmentVariable(L"BEARLIB_LOGLEVEL"), level);
		try_parse(GetEnvironmentVariable(L"BEARLIB_LOGMODE"), mode);
		m_truncated = false;
	}

	void Log::Write(Level level, const std::wstring& what)
	{
		std::wostringstream ss;
		ss << FormatTime().c_str() << " [" << level << "] " << what << std::endl;

		if (filename.empty())
		{
			WriteStandardError(UTF8Encoding().Convert(ss.str()).c_str());
		}
		else
		{
			std::ofstream stream;
			std::ios_base::openmode flags = std::ios_base::out;
			if (mode == Mode::Truncate && !m_truncated)
			{
				flags |= std::ios_base::trunc;
				m_truncated = true;
			}
			else
			{
				flags |= std::ios_base::app;
			}
	#if defined(_MSC_VER)
			// MSVC has wchar_t open overload and does not work well with UTF-8
			stream.open(FixPathSeparators(filename).c_str(), flags);
	#else
			// GCC on the other hand opens UTF-8 paths properly but has no wchar_t overload
			// (as well it shouldn't, there is no such overload in standard)
			stream.open(UTF8Encoding().Convert(FixPathSeparators(filename)), flags);
	#endif
			stream << UTF8Encoding().Convert(ss.str());
		}
	}

	std::wostream& operator<< (std::wostream& s, Log::Level value)
	{
		switch (value)
		{
		case Log::Level::Fatal:
			s << L"fatal";
			break;
		case Log::Level::Error:
			s << L"error";
			break;
		case Log::Level::Warning:
			s << L"warning";
			break;
		case Log::Level::Info:
			s << L"info";
			break;
		case Log::Level::Debug:
			s << L"debug";
			break;
		case Log::Level::Trace:
			s << L"trace";
			break;
		}

		return s;
	}

	std::wistream& operator>> (std::wistream& s, Log::Level& value)
	{
		std::wstring temp;
		s >> temp;

		if (temp == L"trace")
			value = Log::Level::Trace;
		else if (temp == L"debug")
			value = Log::Level::Debug;
		else if (temp == L"info")
			value = Log::Level::Info;
		else if (temp == L"warning")
			value = Log::Level::Warning;
		else if (temp == L"error")
			value = Log::Level::Error;
		else if (temp == L"fatal")
			value = Log::Level::Fatal;
		else
			s.setstate(std::ios_base::badbit);

		return s;
	}

	std::wostream& operator<< (std::wostream& s, Log::Mode value)
	{
		switch (value)
		{
		case Log::Mode::Truncate:
			s << L"truncate";
			break;
		case Log::Mode::Append:
			s << L"append";
			break;
		}

		return s;
	}

	std::wistream& operator>> (std::wistream& s, Log::Mode& value)
	{
		std::wstring temp;
		s >> temp;

		if (temp == L"append")
			value = Log::Mode::Append;
		else if (temp == L"truncate")
			value = Log::Mode::Truncate;
		else
			s.setstate(std::ios_base::badbit);

		return s;
	}
}
