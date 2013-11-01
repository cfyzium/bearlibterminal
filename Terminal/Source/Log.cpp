/*
 * Log.cpp
 *
 *  Created on: Oct 7, 2013
 *      Author: Cfyz
 */

#include "Log.hpp"
#include "Encoding.hpp"

#include <fstream>

namespace BearLibTerminal
{
	Logger g_log;

	Logger::Logger():
		m_level(Level::Trace),
		m_mode(Mode::Truncate),
		m_filename(L"bearlibterminal.log"),
		m_truncated(false)
	{ }

	void Logger::Write(Level level, const std::wstring& what)
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
		stream.open(UTF8->Convert(m_filename), flags);
#endif
		stream << UTF8->Convert(what) << "\n";
	}

	void Logger::SetLevel(Level level)
	{
		// Atomic
		m_level = level;
	}

	void Logger::SetFile(const std::wstring& filename)
	{
		std::lock_guard<std::mutex> guard(m_lock);
		m_filename = filename;
#if defined(_WIN32)
		// Windows version: change slashes to backslashes
		for (auto& c: m_filename) if (c == L'/') c = L'\\';
#endif
		m_truncated = false;
	}

	void Logger::SetMode(Mode mode)
	{
		std::lock_guard<std::mutex> guard(m_lock);
		m_mode = mode;
	}

	std::wstring Logger::GetFile() const
	{
		std::lock_guard<std::mutex> guard(m_lock);
		return m_filename;
	}

	Logger::Mode Logger::GetMode() const
	{
		std::lock_guard<std::mutex> guard(m_lock);
		return m_mode;
	}

	std::wostream& operator<< (std::wostream& s, const Logger::Level& value)
	{
		switch (value)
		{
		case Logger::Level::None:
			s << L"none";
			break;
		case Logger::Level::Fatal:
			s << L"fatal";
			break;
		case Logger::Level::Error:
			s << L"error";
			break;
		case Logger::Level::Warning:
			s << L"warning";
			break;
		case Logger::Level::Info:
			s << L"info";
			break;
		case Logger::Level::Debug:
			s << L"debug";
			break;
		case Logger::Level::Trace:
			s << L"trace";
			break;
		default:
			s << L"n/a";
			break;
		}
		return s;
	}

	std::wistream& operator>> (std::wistream& s, Logger::Level& value)
	{
		std::wstring temp;
		s >> temp;
		if (temp == L"trace") value = Logger::Level::Trace;
		else if (temp == L"debug") value = Logger::Level::Debug;
		else if (temp == L"info") value = Logger::Level::Info;
		else if (temp == L"warning") value = Logger::Level::Warning;
		else if (temp == L"error") value = Logger::Level::Error;
		else if (temp == L"fatal") value = Logger::Level::Fatal;
		else if (temp == L"none") value = Logger::Level::None;
		else value = Logger::Level::Error;
		return s;
	}

	std::wostream& operator<< (std::wostream& s, const Logger::Mode& value)
	{
		switch (value)
		{
		case Logger::Mode::Truncate:
			s << L"truncate";
			break;
		case Logger::Mode::Append:
			s << L"append";
			break;
		default:
			s << L"n/a";
			break;
		}
		return s;
	}

	std::wistream& operator>> (std::wistream& s, Logger::Mode& value)
	{
		std::wstring temp;
		s >> temp;
		if (temp == L"append") value = Logger::Mode::Append;
		else value = Logger::Mode::Truncate;
		return s;
	}
}
