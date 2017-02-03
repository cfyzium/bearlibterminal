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

#include "Platform.hpp"
#include "Encoding.hpp"
#include "Utility.hpp"
#include "Log.hpp"
#include <vector>
#include <fstream>
#include <iostream>

#if defined(_WIN32)
// Force SDK version to XP SP3
#if !defined(WINVER)
#define WINVER 0x502
#define _WIN32_WINNT 0x502
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#if defined(__APPLE__)
#include "CocoaWindow.h"
#endif

#if defined(GetCurrentDirectory)
#undef GetCurrentDirectory
#endif

#if defined(GetEnvironmentVariable)
#undef GetEnvironmentVariable
#endif

namespace BearLibTerminal
{
#if defined(_WIN32)
	const wchar_t kPathSeparator = L'\\';
#else
	const wchar_t kPathSeparator = L'/';
#endif

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
		m_handle = dlopen(UTF8Encoding().Convert(name).c_str(), RTLD_NOW|RTLD_GLOBAL);
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
					//LOG(Info, "Symbol \"" << name.c_str() << "\" was found in \"" << filename << "\""); // FIXME: global object dependency failure

					return module;
				}
			}
		}
		catch (std::exception& e)
		{
			//LOG(Error, "Module enumeration has failed: " << e.what()); // FIXME: global object dependency failure
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

	std::unordered_map<std::wstring, std::weak_ptr<Module>> Module::m_cache;

	std::shared_ptr<Module> Module::Load(std::wstring name)
	{
		try
		{
			auto it = m_cache.find(name);
			if (it != m_cache.end())
			{
				if (auto ret = it->second.lock())
					return ret;
			}

			auto ret = std::make_shared<Module>(name);
			m_cache[name] = ret;
			return ret;
		}
		catch (...)
		{
			return std::shared_ptr<Module>{};
		}
	}

	std::wstring FixPathSeparators(std::wstring name)
	{
#if defined(_WIN32)
		for (auto& c: name)
			if (c == L'/')
				c = L'\\';
#endif
		return std::move(name);
	}

	// FIXME: polymorphic stream use is not guaranteed to be safe!
	// FIXME: MinGW and UTF-8 file name encoding.
	std::unique_ptr<std::istream> OpenFileReading(std::wstring name)
	{
		name = FixPathSeparators(std::move(name));
		std::unique_ptr<std::istream> result;
#if defined(_MSC_VER)
		result.reset(new std::ifstream(name, std::ios_base::in|std::ios_base::binary));
#else
		result.reset(new std::ifstream(UTF8Encoding().Convert(name), std::ios_base::in|std::ios_base::binary));
#endif
		if (result->fail())
		{
			throw std::runtime_error("file \"" + UTF8Encoding().Convert(name) + "\" cannot be opened");
		}

		return result;
	}

	std::unique_ptr<std::ostream> OpenFileWriting(std::wstring name)
	{
		name = FixPathSeparators(std::move(name));
		std::unique_ptr<std::ostream> result;
#if defined(_MSC_VER)
		result.reset(new std::ofstream
			(name, std::ios_base::out|std::ios_base::trunc|std::ios_base::binary));
#else
		result.reset(new std::ofstream
			(UTF8Encoding().Convert(name), std::ios_base::out|std::ios_base::trunc|std::ios_base::binary));
#endif
		if (result->fail())
		{
			throw std::runtime_error("file \"" + UTF8Encoding().Convert(name) + "\" cannot be opened for writing");
		}

		return result;
	}

	std::vector<uint8_t> ReadFile(std::wstring name)
	{
		name = FixPathSeparators(std::move(name));
		//std::unique_ptr<std::istream> result;
#if defined(_MSC_VER)
		std::ifstream file{name, std::ios_base::in|std::ios_base::binary};
#else
		std::ifstream file{UTF8Encoding().Convert(name), std::ios_base::in|std::ios_base::binary};
#endif
		if (file.fail())
			throw std::runtime_error("file \"" + UTF8Encoding().Convert(name) + "\" cannot be opened");

		file.seekg(0, std::ios_base::end);
		size_t size = file.tellg();
		file.seekg(0, std::ios_base::beg);
		std::vector<uint8_t> result(size);
		file.read((char*)&result[0], size);

		LOG(Debug, "Loaded resource from '" << name << "' (" << size << " bytes)");
		return std::move(result);
	}

	bool FileExists(std::wstring name)
	{
#if defined(_WIN32)
		struct _stat st;
		return ::_wstat(name.c_str(), &st) == 0;
#else
		struct stat st;
		return ::stat(UTF8Encoding().Convert(name).c_str(), &st) == 0;
#endif
	}

	std::wstring GetEnvironmentVariable(const std::wstring& name, const std::wstring& default_)
	{
#if defined(_WIN32)
		const DWORD buffer_size = 256;
		WCHAR buffer[buffer_size];
		DWORD rc = ::GetEnvironmentVariableW(name.c_str(), buffer, buffer_size);
		if (rc == 0 || rc > buffer_size)
		{
			// * The buffer is too small? (result is the size required);
			// * Variable not found (GetLastError will be ERROR_ENVVAR_NOT_FOUND);
			// * Or any other reason.
			return default_;
		}
		else
		{
			return std::wstring(buffer);
		}
#else
		const char* p = ::getenv(UTF8Encoding().Convert(name).c_str());
		return p? UTF8Encoding().Convert(p): default_;
#endif
	}

	static std::wstring GetModulePath()
	{
#if defined(_WIN32)
		const DWORD buffer_size = MAX_PATH;
		WCHAR buffer[buffer_size];
		DWORD rc = ::GetModuleFileNameW(nullptr, buffer, buffer_size);
		return rc > 0? std::wstring(): std::wstring{buffer};
#else
		// FIXME: NYI
		return L"."; // FIXME: Retrieve the executable name by following /proc/self/exe symlink.
#endif
	}

	static std::wstring GetModuleName()
	{
#if defined(_WIN32)
		return GetModulePath();
#else
		// Linux version using /proc/self/stat file with the information about
		// currently cunning process.

		std::ifstream stat("/proc/self/stat");
		if (!stat.good())
			return L"";

		int pid;          // The process ID
		std::string comm; // The filename of the executable, in parentheses.

		stat >> pid >> comm;
		if (comm.length() < 3)
			return L"";

		return UTF8Encoding().Convert(comm.substr(1, comm.length()-2));
#endif
	}

	std::wstring GetAppName()
	{
		std::wstring result;

		// Try environment variable first, executable name next.
		if ((result = GetEnvironmentVariable(L"BEARLIB_APPNAME")).empty())
			result = GetModuleName();

		// Cut off path part.
		size_t slash_pos = result.find_last_of(kPathSeparator);
		if (slash_pos != std::wstring::npos)
			result = result.substr(slash_pos+1);

		// Cut off possible extension.
		size_t period_pos = result.find_last_of(L".");
		if (period_pos != std::wstring::npos)
			result = result.substr(0, period_pos);

		if (result.empty())
			result = L"BearLibTerminal";

		return result;
	}

	std::wstring GetAppDirectory()
	{
		std::wstring path;

		std::wstring appname = GetEnvironmentVariable(L"BEARLIB_APPNAME");
		if (!appname.empty())
		{
			// Might be a fully qualified or relative to CWD filename.
			if (FileExists(appname) || FileExists(appname = (GetCurrentDirectory() + appname)))
				path = appname;
		}

		if (path.empty())
		{
			// Retrieve executable location.
			path = GetModulePath();
		}

		// Strip off possible filename portion.
		size_t slash_pos = path.find_last_of(kPathSeparator);
		if (slash_pos != std::wstring::npos)
			path = path.substr(0, slash_pos);

		// If nothing or error occured, fall back to CWD.
		if (path.empty())
		{
			// Failed despite all attempts.
			path = L".";
		}

		if (path.back() != kPathSeparator)
			path += kPathSeparator;

		return path;
	}

	std::wstring GetCurrentDirectory()
	{
		std::wstring result;
#if defined(_WIN32)
		const DWORD buffer_size = MAX_PATH;
		WCHAR buffer[buffer_size];
		if (::GetCurrentDirectoryW(buffer_size, buffer) != 0)
			result = std::wstring{buffer};
#else
		const size_t buffer_size = 1024; // XXX
		char buffer[buffer_size];
		if (::getcwd(buffer, buffer_size) != nullptr)
			result = UTF8Encoding().Convert(buffer);
#endif
		if (result.empty())
			result = L".";
		if (result.back() != kPathSeparator)
			result += kPathSeparator;
		return result;
	}

	std::list<std::wstring> EnumerateFiles(std::wstring path)
	{
		std::list<std::wstring> result;
		if (path.empty())
			return result;

#if defined(_WIN32)
		HANDLE dir;
		WIN32_FIND_DATAW data;

		// Rewrite slashes for better compatibility.
		for (auto& c: path)
		{
			if (c == L'/')
				c = L'\\';
		}

		// Appending '\*' handles the case of querying a disk root.
		if (path.back() != L'\\')
			path += L'\\';
		path += L'*';

		if ((dir = ::FindFirstFileW(path.c_str(), &data)) != INVALID_HANDLE_VALUE)
		{
			BOOL rc = TRUE;
			do
			{
				if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					result.push_back(data.cFileName);
				rc = ::FindNextFileW(dir, &data);
			}
			while (rc);
			::FindClose(dir);
		}
#else
		DIR* dir;

		std::string u8path = UTF8Encoding().Convert(path);
		if (u8path.empty() || u8path.back() != '/')
			u8path += '/';

		if ((dir = ::opendir(u8path.c_str())) != nullptr)
		{
			struct dirent ent, *pent = &ent;
			while (::readdir_r(dir, pent, &pent) == 0 && pent != nullptr)
			{
				struct stat st;
				if (::stat((u8path + pent->d_name).c_str(), &st) == 0 && (st.st_mode & S_IFREG))
				{
					result.push_back(UTF8Encoding().Convert(pent->d_name));
				}
			}
			::closedir(dir);
		}
#endif
		return result;
	}

	void EnsureStandardOutput()
	{
#if defined(_WIN32)
		::AttachConsole(ATTACH_PARENT_PROCESS);
		WriteStandardError("\n");
#endif
	}

	void WriteStandardError(const char* what)
	{
#if defined(_WIN32)
		HANDLE stderr_handle = GetStdHandle(STD_OUTPUT_HANDLE);
		if (stderr_handle == INVALID_HANDLE_VALUE)
			return;

		DWORD written = 0;
		WriteFile(stderr_handle, what, strlen(what), &written, NULL);
#else
		std::cerr << what;
#endif
	}

	std::wstring GetClipboardContents()
	{
#if defined(_WIN32)
		// TODO
		return std::wstring();
#elif defined(__APPLE__)
		return GetCocoaPasteboardString();
#else
		// TODO: X11
		return std::wstring();
#endif
	}
}
