/*
* BearLibTerminal
* Copyright (C) 2013 Cfyz
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

#ifndef BEARLIBTERMINAL_LOG_HPP
#define BEARLIBTERMINAL_LOG_HPP

#include <string>
#include <sstream>

namespace BearLibTerminal
{
	class Log
	{
	public:
		enum class Level {Fatal, Error, Warning, Info, Debug, Trace};
		enum class Mode {Truncate, Append};

	public:
		void Write(Level level, const std::wstring& what);
		void Reset();
		static Log& Instance();
		std::wstring filename;
		Level level;
		Mode mode;

	private:
		Log();
		bool m_truncated;
	};

	std::wostream& operator<< (std::wostream& stream, Log::Level value);
	std::wistream& operator>> (std::wistream& stream, Log::Level& value);

	std::wostream& operator<< (std::wostream& stream, Log::Mode value);
	std::wistream& operator>> (std::wistream& stream, Log::Mode& mode);
}

#define LOG(level_, what)\
	do\
	{\
		if (BearLibTerminal::Log::Level::level_ <= BearLibTerminal::Log::Instance().level)\
		{\
			std::wostringstream wss_;\
			wss_ << what;\
			BearLibTerminal::Log::Instance().Write(BearLibTerminal::Log::Level::level_, wss_.str());\
		}\
	}\
	while (0)

#endif // BEARLIBTERMINAL_LOG_HPP
