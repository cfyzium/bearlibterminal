/*
 * Utility.hpp
 *
 *  Created on: Oct 16, 2013
 *      Author: Cfyz
 */

#ifndef UTILITY_HPP_
#define UTILITY_HPP_

#include <string>
#include <sstream>
#include <mutex>
#include <algorithm>
#include <memory>

namespace BearLibTerminal
{
	template<typename T, typename char_t> bool try_parse(const std::basic_string<char_t>& s, T& out)
	{
		std::basic_stringstream<char_t> stream(s);
		stream >> out;
		return !(stream.fail() || stream.bad());
	}

	template<typename T, typename char_t> bool try_parse(const std::basic_string<char_t>& s)
	{
		T temp;
		return try_parse(s, temp);
	}
	
	template<typename T, typename char_t> T parse(const std::basic_string<char_t>& s)
	{
		T result;
		if (!try_parse(s, result)) result = T();
		return result;
	}
	
	template<typename T, typename char_t> bool try_parse(const std::basic_string<char_t>& s, T& out, std::ios_base& (*f)(std::ios_base&))
	{
		std::basic_istringstream<char_t> stream(s);
		return !(stream >> f >> out).fail();
	}
	
	template<typename T, typename char_t> T parse(const std::basic_string<char_t>& s, std::ios_base& (*f)(std::ios_base&))
	{
		T result;
		if (!try_parse(s, result, f)) result = T();
		return result;
	}

	bool try_parse(const std::wstring& s, bool& out);

	bool try_parse(const std::wstring& s, uint16_t& out);

	bool try_parse(const std::wstring& s, uint64_t& out);

	bool try_parse(const std::wstring& s, wchar_t& out);

	template<typename char_t, typename T> std::basic_string<char_t> to_string(const T& value)
	{
		std::basic_ostringstream<char_t> stream;
		stream << value;
		return stream.str();
	}

	template<typename char_t> bool ends_with(const std::basic_string<char_t>& what, const std::basic_string<char_t>& with)
	{
		return what.rfind(with) == what.length() - with.length();
	}

	template<typename char_t> std::basic_string<char_t> file_extension(const std::basic_string<char_t>& s)
	{
		size_t n = s.find_last_of(char_t('.'));
		if (s.length() > 1 && n < s.length()-1)
		{
			return s.substr(n+1);
		}
		else
		{
			return std::basic_string<char_t>();
		}
	}

	template<typename T> T get_locked(const T& reference, std::mutex& lock)
	{
		std::lock_guard<std::mutex> guard(lock);
		return reference;
	}

	template<typename char_t> std::basic_string<char_t> to_lower(std::basic_string<char_t> s)
	{
		std::transform(s.begin(), s.end(), s.begin(), ::tolower);
		return s;
	}

	uint64_t gettime();

	inline uint64_t start_timing()
	{
		return gettime();
	}

	inline uint64_t stop_timing(uint64_t start)
	{
		return gettime() - start;
	}

	template<typename T> class average
	{
	public:
		average():
			m_sum(),
			m_count()
		{ }

		void add(T value)
		{
			m_sum += value;
			m_count += 1;
		}

		T get()
		{
			T result = m_count? m_sum / m_count: T();
			m_sum = T();
			m_count = 0;
			return result;
		}

	private:
		T m_sum;
		size_t m_count;
	};
}

namespace std
{
	template<typename T, typename... Args> std::unique_ptr<T> make_unique(Args&&... args)
	{
	    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}
}

#endif /* UTILITY_HPP_ */
