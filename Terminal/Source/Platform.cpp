/*
* BearLibTerminal
* Copyright (C) 2014 Cfyz
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

#include "Platform.hpp"
#include "Encoding.hpp"
#include "Log.hpp"
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

namespace BearLibTerminal
{
	Module::Module():
		m_handle(nullptr),
		m_owner(false)
	{ }

	Module::Module(std::wstring name)
	{
#if defined(_WIN32)
		m_handle = (void*)LoadLibraryW(name.c_str());
		if (m_handle == nullptr)
		{
			throw std::runtime_error(""); // FIXME: GetLastError string
		}
#else
		m_handle = dlopen(UTF8->Convert(name).c_str(), RTLD_NOW|RTLD_GLOBAL);
		if (m_handle == nullptr)
		{
			throw std::runtime_error(dlerror());
		}
#endif
		m_owner = true;
	}

	Module::Module(Module&& from):
		m_handle(from.m_handle),
		m_owner(from.m_owner)
	{
		from.m_handle = nullptr;
		from.m_owner = false;
	}

	Module::Module(Handle handle):
		m_handle(handle),
		m_owner(false)
	{ }

	Module::~Module()
	{
		if (m_owner && m_handle)
		{
#if defined(_WIN32)
			FreeLibrary((HMODULE)m_handle);
#else
			dlclose(m_handle);
#endif
		}
	}

	Module::Handle Module::GetHandle() const
	{
		return m_handle;
	}

	Module& Module::operator=(Module&& from)
	{
		std::swap(m_handle, from.m_handle);
		std::swap(m_owner, from.m_owner);
		return *this;
	}

	void* Module::Probe(std::string name) const
	{
		if (m_handle == nullptr)
		{
			throw std::runtime_error("module handle is empty");
		}
#if defined(_WIN32)
		return (void*)GetProcAddress((HMODULE)m_handle, name.c_str());
#else
		return dlsym(m_handle, name.c_str());
#endif
	}

	void* Module::operator[](std::string name) const
	{
		void* p = Probe(std::move(name));

		if (p == nullptr)
		{
#if defined(_WIN32)
			throw std::runtime_error(""); // FIXME: GetLastError string
#else
			throw std::runtime_error(dlerror());
#endif
		}

		return p;
	}

	Module::operator bool() const
	{
		return m_handle != nullptr;
	}

	Module Module::GetProviding(std::string name)
	{
#if defined(_WIN32)
		try
		{
			Module psapi(L"Psapi.dll");
			auto EnumProcessModules = (BOOL WINAPI (*)(HANDLE, HMODULE*, DWORD, LPDWORD))psapi["EnumProcessModules"];
			auto GetModuleFileNameExW = (DWORD WINAPI (*)(HANDLE, HMODULE, LPWSTR, DWORD))psapi["GetModuleFileNameExW"];

			HANDLE process = GetCurrentProcess();
			DWORD bytes_needed = 0;
			EnumProcessModules(process, nullptr, 0, &bytes_needed);
			size_t n_modules = bytes_needed / sizeof(HMODULE);
			std::vector<HMODULE> modules(n_modules);

			EnumProcessModules(process, modules.data(), modules.size()*sizeof(HMODULE), &bytes_needed);
			for (auto m: modules)
			{
				Module module(m);
				if (module.Probe(name))
				{
					wchar_t filename[MAX_PATH];
					GetModuleFileNameExW(process, m, filename, MAX_PATH);
					LOG(Info, "Symbol \"" << name.c_str() << "\" was found in \"" << filename << "\"");

					return module;
				}
			}
		}
		catch (std::exception& e)
		{
			LOG(Error, "Module enumeration has failed: " << e.what());
		}
#else
		void* m = dlopen(0, RTLD_NOW|RTLD_GLOBAL);
		if (dlsym(m, name.c_str()) != nullptr)
		{
			return Module(m);
		}
#endif
		return Module();
	}
}
