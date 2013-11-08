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

#ifndef BEARLIBTERMINAL_TERMINAL_HPP
#define BEARLIBTERMINAL_TERMINAL_HPP

#include "Color.hpp"
#include "Stage.hpp"
#include "Window.hpp"
#include "Options.hpp"
#include "Encoding.hpp"
#include "OptionGroup.hpp"
#include <mutex>
#include <deque>
#include <atomic>
#include <condition_variable>

namespace BearLibTerminal
{
	class Terminal
	{
	public:
		Terminal();
		~Terminal();
		int SetOptions(const std::wstring& value);
		void Refresh();
		void Clear();
		void Clear(int x, int y, int w, int h);
		void SetLayer(int layer_index);
		void SetForeColor(Color color);
		void SetBackColor(Color color);
		void SetComposition(int mode);
		void Put(int x, int y, wchar_t code);
		void PutExtended(int x, int y, int dx, int dy, wchar_t code, Color* corners);
		int Print(int x, int y, const std::wstring& str);
		int HasInput();
		int GetState(int code);
		int Read();
		int ReadChar();
		int ReadString(int x, int y, wchar_t* buffer, int max);
		const Encoding<char>& GetEncoding() const;
	private:
		void SetOptionsInternal(const std::wstring& params);
		void ApplyTilesets(std::map<uint16_t, std::unique_ptr<Tileset>>& tilesets);
		void UpdateDynamicTileset(Size size);
		void ValidateWindowOptions(OptionGroup& group, Options& options);
		void ValidateInputOptions(OptionGroup& group, Options& options);
		void ValidateTerminalOptions(OptionGroup& group, Options& options);
		void ValidateLoggingOptions(OptionGroup& group, Options& options);
		void ConfigureViewport();
		void PutUnlocked(int x, int y, int dx, int dy, wchar_t code, Color* colors);
		void PrepareFreshCharacters();
		void ConsumeIrrelevantInput();
		void ConsumeStroke(const Keystroke& stroke);
		Keystroke ReadKeystroke(int timeout);
		int ReadCharInternal(int timeout);
		int ReadStringInternalBlocking(int x, int y, wchar_t* buffer, int max);
		int ReadStringInternalNonblocking(wchar_t* buffer, int max);
		void OnWindowClose();
		void OnWindowRedraw();
		void OnWindowInput(Keystroke keystroke);
		void OnWindowActivate();
		void InvokeOnRenderingThread(std::function<void()> func);
	private:
		enum state_t {kHidden, kVisible, kClosed} m_state;
		mutable std::mutex m_lock;
		std::unique_ptr<Window> m_window;
		std::deque<Keystroke> m_input_queue;
		std::condition_variable m_input_condvar;
		std::array<std::int32_t, 0x100> m_vars;
		std::unique_ptr<Encoding<char>> m_encoding;
		World m_world;
		Options m_options;
		std::list<uint16_t> m_fresh_codes;
		std::atomic<bool> m_synchronous;
	};
}

#endif // BEARLIBTERMINAL_TERMINAL_HPP
