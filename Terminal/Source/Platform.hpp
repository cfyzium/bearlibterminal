/*
* BearLibTerminal
* Copyright (C) 2014-2016 Cfyz
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

#ifndef BEARLIBTERMINAL_PLATFORM_HPP
#define BEARLIBTERMINAL_PLATFORM_HPP

#include <string>
#include <memory>
#include <istream>
#include <ostream>
#include <vector>
#include <list>
#include <unordered_map>
#include <stdint.h>

#if !defined(_WIN32)
#define __stdcall
#endif

namespace BearLibTerminal
{
	struct cdecl_t;

	struct stdcall_t;

	template<typename R, typename C, typename... Args> struct FunctionSignature;

	template<typename R, typename... Args> struct FunctionSignature<R, cdecl_t, Args...>
	{
		typedef R (*type)(Args...);
	};

	template<typename R, typename... Args> struct FunctionSignature<R, stdcall_t, Args...>
	{
		typedef R(__stdcall *type)(Args...);
	};

	class Module
	{
	public:
		template<typename R, typename C, typename... Args> class Function
		{
		public:
			typedef typename FunctionSignature<R, C, Args...>::type function_type;
			typedef R return_type;

		public:
			Function():
				m_function(nullptr)
			{ }

			Function(std::wstring module_name, std::string symbol_name)
			{
				Load(module_name, symbol_name);
			}

			operator bool()
			{
				return m_function != nullptr;
			}

			bool Load(std::wstring module_name, std::string symbol_name)
			{
				if (auto module = Module::Load(module_name))
				{
					if (auto pointer = module->Probe(symbol_name))
					{
						m_function = reinterpret_cast<function_type>(pointer);
						m_module = module;
						return true;
					}
				}

				return false;
			}

			bool Load(void* pointer)
			{
				if (pointer != nullptr)
				{
					m_function = reinterpret_cast<function_type>(pointer);
					return true;
				}

				return false;
			}

			return_type operator() (Args... args)
			{
				return m_function(args...);
			}

		private:
			function_type m_function;
			std::shared_ptr<Module> m_module;
		};

	public:
		typedef void* Handle;
		Module();
		Module(std::wstring name);
		Module(Module&& from);
		explicit Module(Handle handle);
		~Module();
		Handle GetHandle() const;
		Module& operator=(Module&& from);
		void* operator[](std::string name) const;
		explicit operator bool() const;
		void* Probe(std::string name) const;
		static Module GetProviding(std::string name);
		static std::shared_ptr<Module> Load(std::wstring name);

	private:
		Module(const Module&);
		Module& operator=(const Module&);
		Handle m_handle;
		bool m_owner;
		static std::unordered_map<std::wstring, std::weak_ptr<Module>> m_cache;
	};

	std::wstring FixPathSeparators(std::wstring name);

	std::unique_ptr<std::istream> OpenFileReading(std::wstring name);

	std::unique_ptr<std::ostream> OpenFileWriting(std::wstring name);

	std::vector<uint8_t> ReadFile(std::wstring name);

	std::wstring GetEnvironmentVariable(const std::wstring& name, const std::wstring& default_ = std::wstring());

	bool FileExists(std::wstring name);

	std::wstring GetAppName();

	std::wstring GetAppDirectory();

	std::wstring GetCurrentDirectory();

	std::list<std::wstring> EnumerateFiles(std::wstring path);

	void EnsureStandardOutput();

	void WriteStandardError(const char* what);

	std::wstring GetClipboardContents();
}

#endif // BEARLIBTERMINAL_PLATFORM_HPP
