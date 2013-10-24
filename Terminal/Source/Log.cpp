/*
 * Log.cpp
 *
 *  Created on: Oct 7, 2013
 *      Author: Cfyz
 */

#include "Log.hpp"

#include <fstream>

namespace BearLibTerminal
{
	Log Logger;
//class Log
//{
//public:
//	typedef enum class Level_ {None, Fatal, Error, Warning, Info, Debug, Trace} Level;
//public:
	Log::Log():
		m_level(Level::Trace)
	{ }

	void Log::Write(Level level, const std::wstring& what)
	{
		std::lock_guard<std::mutex> guard(m_lock);
		std::wofstream stream("./bearlibterminal.log", std::ios_base::out|std::ios_base::app);
		stream << what << "\n";
	}

	void Log::SetLevel(Level level)
	{ }

	void Log::SetFile(const std::wstring& filename)
	{ }

	Log::Level Log::GetLevel() const
	{
		std::lock_guard<std::mutex> guard(m_lock);
		return m_level;
	}
//private:
//	mutable std::mutex m_lock;
//	Level m_level;
//	std::wstring m_filename;
//};

	std::wostream& operator<< (std::wostream& stream, const Log::Level& value)
	{
		return stream;
	}

	std::wistream& operator>> (std::wistream& stream, Log::Level& value)
	{
		return stream;
	}
}
