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
#include "Log.hpp"
#include <deque>
#include <array>
#include <thread>

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
		void SetCrop(int x, int y, int w, int h);
		void SetLayer(int layer_index);
		void SetForeColor(Color color);
		void SetBackColor(Color color);
		void SetComposition(int mode);
		void Put(int x, int y, int code);
		void PutExtended(int x, int y, int dx, int dy, int code, Color* corners);
		int Pick(int x, int y, int index);
		Color PickForeColor(int x, int y, int index);
		Color PickBackColor(int x, int y);
		int Print(int x, int y, std::wstring str, bool raw, bool measure_only);
		int HasInput();
		int GetState(int code);
		int Read();
		int ReadString(int x, int y, wchar_t* buffer, int max);
		int Peek();
		void Delay(int period);
		const Encoding8& GetEncoding() const;
	private:
		void SetOptionsInternal(const std::wstring& params);
		void UpdateDynamicTileset(Size size);
		void ValidateWindowOptions(OptionGroup& group, Options& options);
		void ValidateInputOptions(OptionGroup& group, Options& options);
		void ValidateOutputOptions(OptionGroup& group, Options& options);
		void ValidateTerminalOptions(OptionGroup& group, Options& options);
		void ValidateLoggingOptions(OptionGroup& group, Options& options);
		bool ParseInputFilter(const std::wstring& s, std::set<int>& out);
		void ConfigureViewport();
		void PutInternal(int x, int y, int dx, int dy, char32_t code, Color* colors);
		void ConsumeEvent(Event& event);
		Event ReadEvent(int timeout);
		void Render(bool update_scene);
		int Redraw();
		int OnWindowEvent(Event event);
		void PushEvent(Event event);
	private:
		enum state_t {kHidden, kVisible, kClosed} m_state;
		std::thread::id m_main_thread_id;
		std::unique_ptr<Window> m_window;
		std::deque<Event> m_input_queue;
		std::array<int32_t, 256> m_vars;
		std::unique_ptr<Encoding8> m_encoding;
		World m_world;
		Options m_options;
		std::list<char32_t> m_fresh_codes;
		std::map<std::wstring, std::unique_ptr<Encoding8>> m_codepage_cache;
		bool m_show_grid;
		bool m_viewport_modified;
		Rectangle m_viewport_scissors;
		bool m_viewport_scissors_enabled;
		int m_scale_step;
		Rectangle m_stage_area;
		SizeF m_stage_area_factor;
	};
}

#endif // BEARLIBTERMINAL_TERMINAL_HPP
