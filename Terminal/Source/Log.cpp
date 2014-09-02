/*
 * Log.cpp
 *
 *  Created on: Oct 7, 2013
 *      Author: Cfyz
 */

#include "Log.hpp"
#include "Encoding.hpp"

#include <fstream>
#include <time.h>
#include <chrono>

namespace BearLibTerminal
{
	Log* g_logger = nullptr;

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

	Log::Log():
		m_level(Level::Error),
		m_mode(Mode::Truncate),
		m_filename(L"bearlibterminal.log"),
		m_truncated(false)
	{ }

	void Log::Write(Level level, const std::wstring& what)
	{
		std::lock_guard<std::mutex> guard(m_lock);
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
		std::wostringstream ss;
		ss << FormatTime().c_str() << " [" << level << "] " << what;
		stream << UTF8Encoding().Convert(ss.str()) << "\n";
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
		switch (value)
		{
		case Log::Level::None:
			s << L"None";
			break;
		case Log::Level::Fatal:
			s << L"Fatal";
			break;
		case Log::Level::Error:
			s << L"Error";
			break;
		case Log::Level::Warning:
			s << L"Warning";
			break;
		case Log::Level::Info:
			s << L"Info";
			break;
		case Log::Level::Debug:
			s << L"Debug";
			break;
		case Log::Level::Trace:
			s << L"Trace";
			break;
		default:
			s << L"n/a";
			break;
		}
		return s;
	}

	std::wistream& operator>> (std::wistream& s, Log::Level& value)
	{
		std::wstring temp;
		s >> temp;

		if (temp == L"trace")
		{
			value = Log::Level::Trace;
		}
		else if (temp == L"debug")
		{
			value = Log::Level::Debug;
		}
		else if (temp == L"info")
		{
			value = Log::Level::Info;
		}
		else if (temp == L"warning")
		{
			value = Log::Level::Warning;
		}
		else if (temp == L"error")
		{
			value = Log::Level::Error;
		}
		else if (temp == L"fatal")
		{
			value = Log::Level::Fatal;
		}
		else if (temp == L"none")
		{
			value = Log::Level::None;
		}
		else
		{
			s.setstate(std::wistream::badbit);
		}

		return s;
	}

	std::wostream& operator<< (std::wostream& s, const Log::Mode& value)
	{
		switch (value)
		{
		case Log::Mode::Truncate:
			s << L"truncate";
			break;
		case Log::Mode::Append:
			s << L"append";
			break;
		default:
			s << L"n/a";
			break;
		}
		return s;
	}

	std::wistream& operator>> (std::wistream& s, Log::Mode& value)
	{
		std::wstring temp;
		s >> temp;

		if (temp == L"append")
		{
			value = Log::Mode::Append;
		}
		else if (temp == L"truncate")
		{
			value = Log::Mode::Truncate;
		}
		else
		{
			s.setstate(std::wistream::badbit);
		}

		return s;
	}
}
