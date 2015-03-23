/*
 * Log.cpp
 *
 *  Created on: Oct 7, 2013
 *      Author: Cfyz
 */

#include "Log.hpp"
#include "Encoding.hpp"
#include "Utility.hpp"
#include "Platform.hpp"

#include <fstream>
#include <time.h>
#include <chrono>

#include <iostream>
#include "Config.hpp"

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

	Log::Log():
		m_initialized(false),
		m_level(Level::Error),
		m_mode(Mode::Truncate),
		m_filename(),
		m_truncated(false)
	{
		EnsureStandardOutput();
		Init();
	}

	void Log::Init()
	{
		std::wstring s;
		if (!(s = GetEnvironmentVariable(L"BEARLIB_LOGFILE")).empty())
			m_filename = s;

		Level level;
		if (try_parse(GetEnvironmentVariable(L"BEARLIB_LOGLEVEL"), level))
			m_level = level;

		Mode mode;
		if (try_parse(GetEnvironmentVariable(L"BEARLIB_LOGMODE"), mode))
			m_mode = mode;

		m_initialized = true;
	}

	void Log::Dispose()
	{
		std::lock_guard<std::mutex> guard(m_lock);
		m_filename = L"";
		m_level = Level::Error;
		m_mode = Mode::Truncate;
		m_truncated = false;
		m_initialized = false;
	}

	void Log::Write(Level level, const std::wstring& what)
	{
		std::lock_guard<std::mutex> guard(m_lock);

		std::wostringstream ss;
		ss << FormatTime().c_str() << " [" << level << "] " << what << std::endl;

		if (m_filename.empty())
		{
			WriteStandardError(UTF8Encoding().Convert(ss.str()).c_str());
		}
		else
		{
			std::ofstream stream;
			std::ios_base::openmode flags = std::ios_base::out;
			if (m_mode == Mode::Truncate && !m_truncated)
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
			stream.open(m_filename.c_str(), flags);
	#else
			// GCC on the other hand opens UTF-8 paths properly but has no wchar_t overload
			// (as well it shouldn't, there is no such overload in standard)
			stream.open(UTF8Encoding().Convert(m_filename), flags);
	#endif
			stream << UTF8Encoding().Convert(ss.str());
		}
	}

	void Log::SetLevel(Level level)
	{
		// Not atomic but close enough
		m_level = level;
	}

	void Log::SetFile(const std::wstring& filename)
	{
		std::lock_guard<std::mutex> guard(m_lock);
		m_filename = filename;
#if defined(_WIN32)
		// Windows version: change slashes to backslashes
		for (auto& c: m_filename) if (c == L'/') c = L'\\';
#endif
		m_truncated = false;
	}

	void Log::SetMode(Mode mode)
	{
		std::lock_guard<std::mutex> guard(m_lock);
		m_mode = mode;
	}

	std::wstring Log::GetFile() const
	{
		std::lock_guard<std::mutex> guard(m_lock);
		return m_filename;
	}

	Log::Level Log::GetLevel() const
	{
		// Not atomic but close enough
		return m_level;
	}

	Log::Mode Log::GetMode() const
	{
		std::lock_guard<std::mutex> guard(m_lock);
		return m_mode;
	}

	std::wostream& operator<< (std::wostream& s, const Log::Level& value)
	{
		const wchar_t* map[] = {L"none", L"fatal", L"error", L"warning", L"info", L"debug", L"trace"};
		s << map[(int)value];
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
		else if (temp == L"none")
			value = Log::Level::None;
		else
			s.setstate(std::wistream::badbit);

		return s;
	}

	std::wostream& operator<< (std::wostream& s, const Log::Mode& value)
	{
		const wchar_t* map[] = {L"truncate", L"append"};
		s << map[(int)value];
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
			s.setstate(std::wistream::badbit);

		return s;
	}
}
