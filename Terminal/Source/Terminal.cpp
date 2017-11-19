/*
* BearLibTerminal
* Copyright (C) 2013-2017 Cfyz
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

#include "Terminal.hpp"
#include "OpenGL.hpp"
#include "Utility.hpp"
#include "Geometry.hpp"
#include "Log.hpp"
#include "Palette.hpp"
#include "BearLibTerminal.h"
#include <cmath>
#include <future>
#include <vector>
#include <locale.h>

#include <iostream>
#include "Config.hpp"

// Internal usage
#define TK_CLIENT_WIDTH  0xF0
#define TK_CLIENT_HEIGHT 0xF1

//#define DEBUG_TIMING

#if defined(DEBUG_TIMING)
namespace BearLibTerminal
{
	average<float> time_scene_full, time_scene, time_copy, time_invoke, time_draw, time_swap;
	uint64_t time_last_report_time = 0;
}
#endif

namespace BearLibTerminal
{
	std::unique_ptr<Terminal> g_instance;

	static std::vector<float> kScaleSteps =
	{
		0.75f, 1.0f, 1.25f, 1.5f, 2.0f, 3.0f
	};

	static const int kScaleDefault = 1;

	static int GetInputEventNameByName(const std::wstring& name)
	{
		static std::map<std::wstring, int> mapping =
		{
			{L"a", TK_A},
			{L"b", TK_B},
			{L"c", TK_C},
			{L"d", TK_D},
			{L"e", TK_E},
			{L"f", TK_F},
			{L"g", TK_G},
			{L"h", TK_H},
			{L"i", TK_I},
			{L"j", TK_G},
			{L"k", TK_K},
			{L"l", TK_L},
			{L"m", TK_M},
			{L"n", TK_N},
			{L"o", TK_O},
			{L"p", TK_P},
			{L"q", TK_Q},
			{L"r", TK_R},
			{L"s", TK_S},
			{L"t", TK_T},
			{L"u", TK_U},
			{L"v", TK_V},
			{L"w", TK_W},
			{L"x", TK_X},
			{L"y", TK_Y},
			{L"z", TK_Z},
			{L"1", TK_1},
			{L"2", TK_2},
			{L"3", TK_3},
			{L"4", TK_4},
			{L"5", TK_5},
			{L"6", TK_6},
			{L"7", TK_7},
			{L"8", TK_8},
			{L"9", TK_9},
			{L"0", TK_0},
			{L"return", TK_ENTER},
			{L"enter", TK_ENTER},
			{L"escape", TK_ESCAPE},
			{L"backspace", TK_BACKSPACE},
			{L"tab", TK_TAB},
			{L"space", TK_SPACE},
			{L"minus", TK_MINUS},
			{L"equals", TK_EQUALS},
			{L"lbracket", TK_LBRACKET},
			{L"rbracket", TK_RBRACKET},
			{L"backslash", TK_BACKSLASH},
			{L"semicolon", TK_SEMICOLON},
			{L"apostrophe", TK_APOSTROPHE},
			{L"grave", TK_GRAVE},
			{L"comma", TK_COMMA},
			{L"period", TK_PERIOD},
			{L"slash", TK_SLASH},
			{L"f1", TK_F1},
			{L"f2", TK_F2},
			{L"f3", TK_F3},
			{L"f4", TK_F4},
			{L"f5", TK_F5},
			{L"f6", TK_F6},
			{L"f7", TK_F7},
			{L"f8", TK_F8},
			{L"f9", TK_F9},
			{L"f10", TK_F10},
			{L"f11", TK_F11},
			{L"f12", TK_F12},
			{L"pause", TK_PAUSE},
			{L"insert", TK_INSERT},
			{L"home", TK_HOME},
			{L"pageup", TK_PAGEUP},
			{L"delete", TK_DELETE},
			{L"end", TK_END},
			{L"pagedown", TK_PAGEDOWN},
			{L"right", TK_RIGHT},
			{L"left", TK_LEFT},
			{L"down", TK_DOWN},
			{L"up", TK_UP},
			{L"kp-divide", TK_KP_DIVIDE},
			{L"kp-multiply", TK_KP_MULTIPLY},
			{L"kp-minus", TK_KP_MINUS},
			{L"kp-plus", TK_KP_PLUS},
			{L"kp-enter", TK_KP_ENTER},
			{L"kp-1", TK_KP_1},
			{L"kp-2", TK_KP_2},
			{L"kp-3", TK_KP_3},
			{L"kp-4", TK_KP_4},
			{L"kp-5", TK_KP_5},
			{L"kp-6", TK_KP_6},
			{L"kp-7", TK_KP_7},
			{L"kp-8", TK_KP_8},
			{L"kp-9", TK_KP_9},
			{L"kp-0", TK_KP_0},
			{L"kp-period", TK_KP_PERIOD},
			{L"shift", TK_SHIFT},
			{L"control", TK_CONTROL},
			{L"alt", TK_ALT},
			{L"mouse-left", TK_MOUSE_LEFT},
			{L"mouse-right", TK_MOUSE_RIGHT},
			{L"mouse-middle", TK_MOUSE_MIDDLE},
			{L"mouse-x1", TK_MOUSE_X1},
			{L"mouse-x2", TK_MOUSE_X2},
			{L"mouse-move", TK_MOUSE_MOVE},
			{L"mouse-scroll", TK_MOUSE_SCROLL},
			{L"close", TK_CLOSE},
			{L"resized", TK_RESIZED}
		};

		auto i = mapping.find(name);
		return i == mapping.end()? 0: i->second;
	}

	static std::wstring Escape(const std::wstring& s)
	{
		std::wstring result = L"'";
		for (auto c: s)
		{
			result += c;
			if (c == L'\'')
				result += c;
		}
		result += L"'";
		return result;
	}

	Terminal::Terminal():
		m_state{kHidden},
		m_show_grid{false},
		m_viewport_modified{false},
		m_scale_step(kScaleDefault),
		m_alt_pressed(false)
	{
#if defined(__APPLE__)
		// OS X implementation of C-string manipulation routines (e. g. swprintf)
		// do not accept wide characters (i. e. wide strings at all) unless
		// the character classification locale is set to UTF-8. And default one is "C".
		if (setlocale(LC_CTYPE, "UTF-8") == nullptr)
			LOG(Error, "Failed to set LC_CTYPE locale to UTF-8");
#endif

		// Save main thread ID so we can catch threading violations.
		m_main_thread_id = std::this_thread::get_id();

		// Elements of std::array are not default-initialized.
		for (auto& var: m_vars)
			var = 0;

		// Synchronize log settings (logger reads them from config file but they may be left default).
		m_options.log_filename = Log::Instance().filename;
		m_options.log_level = Log::Instance().level;
		m_options.log_mode = Log::Instance().mode;

		// Try to create window
		m_window = Window::Create(std::bind(&Terminal::OnWindowEvent, this, std::placeholders::_1));

		// Default parameters
		SetOptionsInternal(L"window: size=80x25, icon=default; font: default; terminal.encoding=utf8; input.filter={keyboard}");

		// Apply parameters from configuration file:
		// Each group (line) is applied separately to allow some error resilience.
		LOG(Info, "Applying options from configuration file, if any");
		std::map<std::wstring, std::wstring> groups;
		for (auto& pair: Config::Instance().List(L"ini.bearlibterminal"))
		{
			// TODO: use some more readable "split" function
			std::wstring group = pair.first.substr(0, pair.first.find(L'.'));
			std::wstring property = group.length() >= pair.first.length()-1?
				L"_": pair.first.substr(group.length()+1);
			groups[group] += group + L"." + property + L"=" + Escape(pair.second) + L";";
		}
		for (auto& pair: groups)
			SetOptions(pair.second);
		LOG(Info, "Applying palette from configuration file");
		for (auto& pair: Config::Instance().List(L"ini.palette"))
		{
			Color value = Palette::Instance.Get(pair.second);
			LOG(Info, "* '" << pair.first << "' = '" << pair.second << "' (a=" << (int)value.a << ", r=" << (int)value.r << ", g=" << (int)value.g << ", b=" << (int)value.b << ")");
			Palette::Instance.Set(pair.first, value);
		}
		LOG(Info, "Terminal initialization complete");
	}

	Terminal::~Terminal()
	{
		g_codespace.clear();
		g_tilesets.clear();
		g_atlas.Clear();

		// Window will be disposed of automatically.
	}

	// NOTE: not every API function checks this, only functions dealing with configuration,
	// input and rendering: set, refresh, has_input, read, read_str, peek and delay.
	// NOTE: this introduces an unlikely data race which should be acceptable in this particular case.
	#define CHECK_THREAD(name, ret) \
	if (m_state == kClosed) { \
		return ret; \
	} else if (std::this_thread::get_id() != m_main_thread_id) { \
		LOG(Fatal, "'" name "' was not called from the main thread"); \
		m_state = kClosed; \
		return ret; \
	}

	int Terminal::SetOptions(const std::wstring& value)
	{
		CHECK_THREAD("set", 0);

		// XXX: hack.
		if (value.find(L"log(") == 0)
		{
			if (value.length() > 7) // Seven because quotes.
				LOG(Info, value.substr(5, value.length() - 7));
			return 1;
		}

		LOG(Info, "Trying to set \"" << value << "\"");
		try
		{
			SetOptionsInternal(value);
			return 1;
		}
		catch (std::exception& e)
		{
			LOG(Error, "Failed to set some options: " << e.what());
			return 0;
		}
	}

	std::map<std::wstring, int> g_fonts;

	int AllocateFontIndex(std::wstring name, std::map<std::wstring, int>& preallocated_fonts)
	{
		// Clean up.
		for (auto i = g_fonts.begin(); i != g_fonts.end(); )
		{
			char32_t font_offset = i->second * 0x1000000;
			auto j = g_tilesets.lower_bound(font_offset);
			if (j == g_tilesets.end() || (j->first & Tileset::kFontOffsetMask) != font_offset)
			{
				// The first tileset with offset >= font_offset does not belong to this font.
				// Meaning there is no tilesets belonging to this font.
				i = g_fonts.erase(i);
			}
			else
			{
				i++;
			}
		}

		auto contains_value = [](std::map<std::wstring, int>& map, int value)
		{
			for (auto kv: map)
			{
				if (kv.second == value)
				{
					return true;
				}
			}

			return false;
		};

		// Look up first available index.
		for (int i = 0; ; i++)
		{
			if (!contains_value(preallocated_fonts, i) && !contains_value(g_fonts, i))
			{
				LOG(Info, "New font '" << name << "' -> index " << i);
				preallocated_fonts[name] = i;
				return i;
			}
		}

		return -1;
	}

	char32_t ParseTilesetOffset(std::wstring name, std::map<std::wstring, int>& preallocated_fonts)
	{
		char32_t font_offset = 0;
		std::wstring font_name = L"main";
		size_t space_pos = name.find(L' ');
		if (space_pos != std::wstring::npos && space_pos < name.length()-1)
		{
			font_name = name.substr(0, space_pos);
			name = name.substr(space_pos+1);
		}

		std::map<std::wstring, int>::iterator i;
		if ((i = g_fonts.find(font_name)) != g_fonts.end() ||
		    (i = preallocated_fonts.find(font_name)) != preallocated_fonts.end())
		{
			font_offset = i->second * Tileset::kFontOffsetMultiplier;
		}
		else
		{
			font_offset = AllocateFontIndex(font_name, preallocated_fonts) * Tileset::kFontOffsetMultiplier;
		}

		if (name == L"font")
		{
			return font_offset;
		}

		char32_t tileset_offset = 0;
		if (!try_parse(name, tileset_offset))
		{
			throw std::runtime_error("Failed to parse tileset offset from '" + UTF8Encoding().Convert(name) + "'");
		}

		return font_offset + tileset_offset;
	}

	TileInfo* GetTileInfo(char32_t code)
	{
		auto i = g_codespace.find(code);
		if (i != g_codespace.end())
			return i->second.get();

		char32_t font_low = (code & Tileset::kFontOffsetMask);
		char32_t font_high = font_low + Tileset::kCharOffsetMask;

		for (auto j = g_tilesets.rbegin(); j != g_tilesets.rend(); ++j)
		{
			if (j->first < font_low || j->first > font_high)
			{
				continue;
			}

			if (j->second->Provides(code))
			{
				auto tile = j->second->Get(code);
				g_codespace[code] = tile;
				g_atlas.Add(tile);
				return tile.get();
			}
		}

		if (IsDynamicTile(code))
		{
			if (g_dynamic_tileset)
			{
				auto tile = g_dynamic_tileset->Get(code);
				g_codespace[code] = tile;
				g_atlas.Add(tile);
				return tile.get();
			}
			else
			{
				return nullptr;
			}
		}
		else
		{
			return GetTileInfo(font_low + kUnicodeReplacementCharacter);
		}
	}

	void Terminal::SetOptionsInternal(const std::wstring& value)
	{
		auto groups = ParseOptions2(value);
		Options updated = m_options;
		std::unordered_map<char32_t, std::shared_ptr<Tileset>> new_tilesets;
		std::unordered_map<std::wstring, Color> palette_update;
		std::map<std::wstring, int> preallocated_fonts;

		// Validate options
		for (auto& group: groups)
		{
			LOG(Debug, L"Group \"" << group.name << "\":");

			for (auto attribute: group.attributes)
			{
				LOG(Debug, L"* \"" << attribute.first << "\" = \"" << attribute.second << "\"");
			}

			if (group.name == L"window")
			{
				ValidateWindowOptions(group, updated);
			}
			else if (group.name == L"input")
			{
				ValidateInputOptions(group, updated);
			}
			else if (group.name == L"output")
			{
				ValidateOutputOptions(group, updated);
			}
			else if (group.name == L"terminal")
			{
				ValidateTerminalOptions(group, updated);
			}
			else if (group.name == L"log")
			{
				ValidateLoggingOptions(group, updated);
			}
			else if (starts_with(group.name, std::wstring(L"ini.")))
			{
				for (auto& i: group.attributes)
				{
					// XXX: Just use section-property-value
					Config::Instance().Set(group.name + L"." + i.first, i.second);
				}
			}
			else if (group.name == L"palette")
			{
				for (auto kv: group.attributes)
				{
					palette_update[kv.first] = Palette::Instance.Get(kv.second);
				}
			}
			else
			{
				char32_t offset = ParseTilesetOffset(group.name, preallocated_fonts);
				if (group.attributes[L"_"] == L"none")
				{
					// Remove tileset.
					new_tilesets[offset].reset();
				}
				else
				{
					// Add new tileset.
					group.name = to_string<wchar_t>(offset);
					new_tilesets[offset] = Tileset::Create(group, offset);
				}
			}
		}

		if (updated.output_vsync != m_options.output_vsync)
		{
			m_viewport_modified = true;
		}

		if (updated.output_texture_filter != m_options.output_texture_filter)
		{
			g_texture_filter = updated.output_texture_filter;
		}

		// All options and parameters must be validated, may try to apply them
		for (auto& kv: preallocated_fonts)
		{
			g_fonts[kv.first] = kv.second;
		}
		for (auto& kv: new_tilesets)
		{
			RemoveTileset(kv.first);
			if (kv.second)
				AddTileset(kv.second);
		}
		g_atlas.Defragment();
		g_atlas.CleanUp();

		// Primary sanity check: if there is no base font, lots of things are gonna fail
		if (!g_tilesets.count(0))
			throw std::runtime_error("No main font has been configured");

		// Apply palette
		for (auto kv: palette_update)
			Palette::Instance.Set(kv.first, kv.second);

		Log::Instance().filename = updated.log_filename;
		Log::Instance().level = updated.log_level;
		Log::Instance().mode = updated.log_mode;

		if (updated.terminal_encoding != m_options.terminal_encoding)
		{
			m_encoding = GetUnibyteEncoding(updated.terminal_encoding);
		}

		if (updated.input_mouse_cursor != m_options.input_mouse_cursor)
		{
			m_window->SetCursorVisibility(updated.input_mouse_cursor);
			// FIXME: NYI
		}

		// Apply on per-option basis
		bool viewport_size_changed = false;
		bool cell_size_changed = false;

		if (updated.window_title != m_options.window_title)
		{
			m_window->SetTitle(updated.window_title);
		}

		if (updated.window_icon != m_options.window_icon)
		{
			m_window->SetIcon(updated.window_icon);
		}

		if (updated.window_resizeable != m_options.window_resizeable)
		{
			// XXX: It's not always possible to change resizeability in runtime.
			m_window->SetResizeable(updated.window_resizeable);

			if (updated.window_resizeable)
			{
				// User resize cancels client-size
				// This one handles client-size set before resizeable
				updated.window_client_size = Size();
			}
		}

		// If the size of cell has changed -OR- new basic tileset has been specified
		if (updated.window_cellsize != m_options.window_cellsize || new_tilesets.count(0))
		{
			cell_size_changed = true;

			// Refresh stage.state.cellsize
			m_world.state.cellsize = updated.window_cellsize;

			// If specified cellsize is nil, fall back on base font bounding box
			if (!m_world.state.cellsize.Area())
			{
				// NOTE: by now, tileset container MUST have 0th tileset since
				// one is added in ctor and cannot be fully removed afterwards.
				//m_world.state.cellsize = m_world.tilesets[0]->GetBoundingBoxSize();
				m_world.state.cellsize = g_tilesets[0]->GetBoundingBoxSize();
			}

			// Cache half cellsize
			m_world.state.half_cellsize = Size
			(
				m_world.state.cellsize.width / 2,
				m_world.state.cellsize.height / 2
			);

			// Update state
			m_vars[TK_CELL_WIDTH] = m_world.state.cellsize.width; // TODO: move vars to world.state
			m_vars[TK_CELL_HEIGHT] = m_world.state.cellsize.height;

			// Must readd dynamic tileset.
			// NOTE: SHOULD NOT fail.
			UpdateDynamicTileset(m_world.state.cellsize);

			viewport_size_changed = true;
			LOG(Debug, L"SetOptions: new cell size is " << m_world.state.cellsize);
		}

		// Manual size modification cancels client-size
		if (updated.window_size != m_options.window_size)
		{
			updated.window_client_size = Size();
		}

		if (updated.window_client_size != m_options.window_client_size ||
		   (m_options.window_client_size.Area() > 0 && cell_size_changed)) // cellsize changed when client-size is already set
		{
			if (updated.window_client_size.Area() > 0)
			{
				auto sizef = updated.window_client_size/m_world.state.cellsize.As<float>();
				updated.window_size = std::floor(sizef).As<int>();
				if (updated.window_size.Area() <= 0)
					updated.window_size = Size(1, 1);
			}

			m_options.window_client_size = updated.window_client_size;
			viewport_size_changed = true;
		}

		if (updated.window_size != m_options.window_size)
		{
			// Update window size: resize the stage
			m_world.stage.Resize(updated.window_size);
			m_vars[TK_WIDTH] = m_world.stage.size.width;
			m_vars[TK_HEIGHT] = m_world.stage.size.height;
			viewport_size_changed = true;
			LOG(Debug, L"SetOptions: new window size is " << updated.window_size);
		}

		if (updated.window_minimum_size != m_options.window_minimum_size)
		{
			viewport_size_changed = true; // Hack
		}

		if (viewport_size_changed)
		{
			// Resize window object
			float scale_factor = kScaleSteps[m_scale_step];
			Size viewport_size = m_options.window_client_size.Area() > 0? // client-size overrides viewport dimensions
				m_options.window_client_size:
				m_world.stage.size * m_world.state.cellsize * scale_factor;
			m_vars[TK_CLIENT_WIDTH] = viewport_size.width;
			m_vars[TK_CLIENT_HEIGHT] = viewport_size.height;

			m_window->SetSizeHints(m_world.state.cellsize*scale_factor, updated.window_minimum_size);
			m_window->SetClientSize(viewport_size);

			m_viewport_modified = true;
		}

		if (updated.window_fullscreen != m_options.window_fullscreen)
		{
			// XXX: It's not always possible to change fullscreen state in runtime.
			m_window->SetFullscreen(updated.window_fullscreen);
		}

		m_options = updated;

		// Synchronize options struct with configuration cache (sys.group.option).
		auto bool_to_wstring = [](bool flag) {return flag? L"true": L"false";};
		auto size_to_wstring = [](Size size) {return size.Area()? to_string<wchar_t>(size): std::wstring(L"auto");};
		auto& C = Config::Instance();
		// terminal
		C.Set(L"terminal.encoding", m_options.terminal_encoding);
		C.Set(L"terminal.encoding-affects-put", bool_to_wstring(m_options.terminal_encoding_affects_put));
		// window
		C.Set(L"window.size", size_to_wstring(m_options.window_size));
		C.Set(L"window.cellsize", size_to_wstring(m_options.window_cellsize));
		C.Set(L"window.client-size", size_to_wstring(m_options.window_client_size));
		C.Set(L"window.title", m_options.window_title);
		C.Set(L"window.icon", m_options.window_icon);
		C.Set(L"window.resizeable", bool_to_wstring(m_options.window_resizeable));
		C.Set(L"window.minimum-size", size_to_wstring(m_options.window_minimum_size));
		C.Set(L"window.fullscreen", bool_to_wstring(m_options.window_fullscreen));
		// input
		C.Set(L"input.precise-mouse", bool_to_wstring(m_options.input_precise_mouse));
		//C.Set(L"input.filter", m_options.input_filter_str); // FIXME
		C.Set(L"input.cursor-symbol", std::wstring(1, (wchar_t)m_options.input_cursor_symbol));
		C.Set(L"input.cursor-blink-rate", to_string<wchar_t>(m_options.input_cursor_blink_rate));
		C.Set(L"input.mouse-cursor", bool_to_wstring(m_options.input_mouse_cursor));
		C.Set(L"input.alt-functions", bool_to_wstring(m_options.input_alt_functions));
		// output
		C.Set(L"output.vsync", bool_to_wstring(m_options.output_vsync));
		// log
		C.Set(L"input.file", m_options.log_filename);
		C.Set(L"input.level", to_string<wchar_t>(m_options.log_level));
		C.Set(L"input.mode", to_string<wchar_t>(m_options.log_mode));
	}

	void Terminal::ValidateTerminalOptions(OptionGroup& group, Options& options)
	{
		// Possible options: encoding

		if (group.attributes.count(L"encoding"))
		{
			options.terminal_encoding = group.attributes[L"encoding"];
		}

		if (group.attributes.count(L"encoding-affects-put"))
		{
			try_parse(group.attributes[L"encoding-affects-put"], options.terminal_encoding_affects_put);
		}
	}

	void Terminal::ValidateWindowOptions(OptionGroup& group, Options& options)
	{
		// Possible options: size, cellsize, title, icon

		if (group.attributes.count(L"size"))
		{
			Size value;

			if (!try_parse(group.attributes[L"size"], value))
			{
				throw std::runtime_error("window.size value cannot be parsed");
			}

			if (value.width <= 0 || value.width >= 1024 || value.height <= 0 || value.height >= 1024)
			{
				throw std::runtime_error("window.size value is out of range");
			}

			options.window_size = value;
		}

		if (group.attributes.count(L"cellsize"))
		{
			Size value;

			if (group.attributes[L"cellsize"] != L"auto" && !try_parse(group.attributes[L"cellsize"], value))
			{
				throw std::runtime_error("window.cellsize value cannot be parsed");
			}

			if (value.width < 0 || value.height < 0 || value.width > 64 || value.height > 64)
			{
				throw std::runtime_error("window.cellsize value is out of range");
			}

			options.window_cellsize = value;
		}

		if (group.attributes.count(L"client-size"))
		{
			Size value;

			if (group.attributes[L"client-size"] != L"auto" && !try_parse(group.attributes[L"client-size"], value))
			{
				throw std::runtime_error("window.client-size value cannot be parsed");
			}

			if (value.width < 0 || value.height < 0)
			{
				throw std::runtime_error("window.client-size value is out of range");
			}

			options.window_client_size = value;
		}

		if (group.attributes.count(L"title"))
		{
			options.window_title = group.attributes[L"title"];
		}

		if (group.attributes.count(L"icon"))
		{
			options.window_icon = group.attributes[L"icon"];
		}

		if (group.attributes.count(L"resizeable") && !try_parse(group.attributes[L"resizeable"], options.window_resizeable))
		{
			throw std::runtime_error("window.resizeable value cannot be parsed");
		}

		if (group.attributes.count(L"minimum-size") && !try_parse(group.attributes[L"minimum-size"], options.window_minimum_size))
		{
			throw std::runtime_error("window.minimum-size value cannot be parsed");
		}

		if (options.window_minimum_size.width < 1 || options.window_minimum_size.height < 1)
		{
			throw std::runtime_error("window.minimum-size value is out of range");
		}

		if (group.attributes.count(L"fullscreen"))
		{
			if (!try_parse(group.attributes[L"fullscreen"], options.window_fullscreen))
			{
				throw std::runtime_error("window.fullscreen value cannot be parsed");
			}
		}
	}

	void Terminal::ValidateInputOptions(OptionGroup& group, Options& options)
	{
		// Possible options: nonblocking, events, precise_mouse, cursor_symbol, cursor_blink_rate

		if (group.attributes.count(L"precise-mouse") && !try_parse(group.attributes[L"precise-mouse"], options.input_precise_mouse))
		{
			throw std::runtime_error("input.precise-mouse cannot be parsed");
		}

		if (group.attributes.count(L"filter") && !ParseInputFilter(group.attributes[L"filter"], options.input_filter))
		{
			throw std::runtime_error("input.filter cannot be parsed");
		}

		if (group.attributes.count(L"cursor-symbol") && !try_parse(group.attributes[L"cursor-symbol"], options.input_cursor_symbol))
		{
			throw std::runtime_error("input.cursor-symbol cannot be parsed");
		}

		if (group.attributes.count(L"cursor-blink-rate") && !try_parse(group.attributes[L"cursor-blink-rate"], options.input_cursor_blink_rate))
		{
			throw std::runtime_error("input.cursor-blink-rate cannot be parsed");
		}

		if (options.input_cursor_blink_rate < 0)
			options.input_cursor_blink_rate = 0;

		if (group.attributes.count(L"mouse-cursor") && !try_parse(group.attributes[L"mouse-cursor"], options.input_mouse_cursor))
		{
			throw std::runtime_error("input.mouse-cursor cannot be parsed");
		}

		if (group.attributes.count(L"alt-functions") && !try_parse(group.attributes[L"alt-functions"], options.input_alt_functions))
		{
			throw std::runtime_error("input.alt-functions cannot be parsed");
		}
	}

	bool Terminal::ParseInputFilter(const std::wstring& s, std::set<int>& out)
	{
		// s: list of case-insensitive input event names separated by a comma.
		// macro names: keyboard, mouse

		std::set<int> result;

		auto add = [&result](int code, bool release_too)
		{
			result.insert(code);

			if (release_too)
				result.insert(code | TK_KEY_RELEASED);
		};

		for (size_t start = 0, end = 0; start < s.length(); )
		{
			end = s.find(L",", start);
			if (end == std::wstring::npos) end = s.length();

			if (end > start)
			{
				std::wstring name = trim(s.substr(start, end-start));
				for (auto& c: name) if (c == L'_') c = L'-';

				bool release_too = false;
				if (!name.empty() && name.back() == L'+')
				{
					release_too = true;
					name.resize(name.length()-1);
				}

				if (!name.empty())
				{
					if (name == L"false" || name == L"none")
					{
						result.clear();
					}
					else if (name == L"keyboard")
					{
						for (int i = TK_A; i <= TK_ALT; i++)
							add(i, release_too);
					}
					else if (name == L"arrows")
					{
						for (int i = TK_RIGHT; i <= TK_UP; i++)
							add(i, release_too);
					}
					else if (name == L"keypad")
					{
						for (int i = TK_KP_DIVIDE; i <= TK_KP_PERIOD; i++)
							add(i, release_too);
					}
					else if (name == L"mouse")
					{
						for (int i = TK_MOUSE_LEFT; i <= TK_MOUSE_X2; i++)
							add(i, release_too);

						add(TK_MOUSE_MOVE, false);
						add(TK_MOUSE_SCROLL, false);
					}
					else if (int code = GetInputEventNameByName(name))
					{
						add(code, release_too);
					}
					else
					{
						// Maybe, shortened list of alphanumeric keys?
						bool correct = true;

						for (auto& c: name) // TODO: shorter check
						{
							if (!::isalpha((int)c) && !::isdigit((int)c))
							{
								correct = false;
								break;
							}
						}

						if (correct)
						{
							for (auto& c: name)
							{
								add(GetInputEventNameByName(std::wstring(1, c)), release_too);
							}
						}
					}
				}
			}

			start = end+1;
		}

		if (!result.empty())
		{
			result.insert(TK_CLOSE);
			result.insert(TK_RESIZED);
		}

		out = result;

		return true;
	}

	void Terminal::ValidateOutputOptions(OptionGroup& group, Options& options)
	{
		// Possible options: postformatting, vsync, tab-width

		// TODO: deprecated
		if (group.attributes.count(L"postformatting") && !try_parse(group.attributes[L"postformatting"], options.output_postformatting))
		{
			throw std::runtime_error("output.postformatting cannot be parsed");
		}

		if (group.attributes.count(L"vsync") && !try_parse(group.attributes[L"vsync"], options.output_vsync))
		{
			throw std::runtime_error("output.vsync cannot be parsed");
		}

		if (group.attributes.count(L"tab-width") && !try_parse(group.attributes[L"tab-width"], options.output_tab_width))
		{
			throw std::runtime_error("output.tab-width cannot be parsed");
		}

		if (options.output_tab_width < 0)
			options.output_tab_width = 0;

		if (group.attributes.count(L"texture-filter"))
		{
			if (group.attributes[L"texture-filter"] == L"linear")
				options.output_texture_filter = GL_LINEAR;
			else if (group.attributes[L"texture-filter"] == L"nearest")
				options.output_texture_filter = GL_NEAREST;
			else
				throw std::runtime_error("output.texture-filter cannot be parsed");
		}
	}

	void Terminal::ValidateLoggingOptions(OptionGroup& group, Options& options)
	{
		// Possible options: file, level, mode

		if (group.attributes.count(L"file"))
		{
			options.log_filename = group.attributes[L"file"];
		}

		if (group.attributes.count(L"level") && !try_parse(group.attributes[L"level"], options.log_level))
		{
			throw std::runtime_error("log.level cannot be parsed");
		}

		if (group.attributes.count(L"mode") && !try_parse(group.attributes[L"mode"], options.log_mode))
		{
			throw std::runtime_error("log.mode cannot be parsed");
		}
	}

#if defined(DEBUG_TIMING)
	void Terminal::Refresh()
	{
		static uint64_t time_scene_prev = gettime();
		uint64_t time_scene_now = gettime();
		uint64_t scene_full = time_scene_now - time_scene_prev;
		time_scene_prev = time_scene_now;

		if (m_state == kHidden)
		{
			m_window->Show();
			m_state = kVisible;
		}

		if (m_state != kVisible) return;

		uint64_t time_copy_start;
		// Synchronously copy backbuffer to frontbuffer
		{
			std::lock_guard<std::mutex> guard(m_lock);
			m_world.stage.frontbuffer = m_world.stage.backbuffer;
		}

		uint64_t time_invoke_start = gettime(), time_draw_start, time_swap_start, time_swap_end;
		m_window->Invoke([&]()
		{
			time_draw_start = gettime();
			OnWindowRedraw();
			time_swap_start = gettime();
			m_window->SwapBuffers();
			time_swap_end = gettime();
		});
		uint64_t time_invoke_end = gettime();

		int64_t copy = time_invoke_start - time_copy_start;
		int64_t draw = time_swap_start - time_draw_start;
		int64_t swap = time_swap_end - time_swap_start;
		int64_t invoke = time_invoke_end - time_invoke_start - (draw + swap);

		copy = copy > 0? copy: 0;
		draw = draw > 0? draw: 0;
		swap = swap > 0? swap: 0;
		invoke = invoke > 0? invoke: 0;
		int64_t scene = ((int64_t)scene_full - (int64_t)(copy + draw + swap + invoke));
		scene = scene > 0? scene: 0;

		time_scene_full.add(scene_full);
		time_scene.add(scene);
		time_copy.add(copy);
		time_invoke.add(invoke);
		time_draw.add(draw);
		time_swap.add(swap);

		uint64_t now = gettime();
		if (now > time_last_report_time + 1000000)
		{
			LOG(Trace, "Timing report: full scene " << time_scene_full.get() << ", bare scene " << time_scene.get() << ", copy " << time_copy.get() << ", invoke " << time_invoke.get() << ", draw " << time_draw.get() << ", swap " << time_swap.get());
			time_last_report_time = now;
		}
	}
#else
	void Terminal::Refresh()
	{
		CHECK_THREAD("refresh", );

		if (m_state == kHidden)
		{
			m_window->Show();
			m_state = kVisible;
		}

		m_world.stage.frontbuffer = m_world.stage.backbuffer;
		m_window->PumpEvents();
		Render();
	}
#endif

	void Terminal::Clear()
	{
		if (m_world.stage.backbuffer.background.size() != m_world.stage.size.Area())
		{
			LOG(Trace, "World resize");
			m_world.stage.Resize(m_world.stage.size);
		}
		else
		{
			for (auto& layer: m_world.stage.backbuffer.layers)
			{
				for (auto& cell: layer.cells)
				{
					cell.leafs.clear();
				}

				layer.crop = Rectangle();
			}
		}

		for (auto& color: m_world.stage.backbuffer.background)
		{
			color = m_world.state.bkcolor;
		}
	}

	void Terminal::Clear(int x, int y, int w, int h)
	{
		Size stage_size = m_world.stage.size;
		if (x < 0) x = 0;
		if (y < 0) y = 0;
		if (x+w >= stage_size.width) w = stage_size.width-x;
		if (y+h >= stage_size.height) h = stage_size.height-y;

		Layer& layer = m_world.stage.backbuffer.layers[m_world.state.layer];
		for (int i=x; i<x+w; i++)
		{
			for (int j=y; j<y+h; j++)
			{
				int k = stage_size.width*j+i;
				layer.cells[k].leafs.clear();
				if (m_world.state.layer == 0)
				{
					m_world.stage.backbuffer.background[k] = m_world.state.bkcolor;
				}
			}
		}
	}

	void Terminal::SetCrop(int x, int y, int w, int h)
	{
		m_world.stage.backbuffer.layers[m_world.state.layer].crop =
			Rectangle(m_world.stage.size).Intersection(Rectangle(x, y, w, h));
	}

	void Terminal::SetLayer(int layer_index)
	{
		// Layer index is limited to [0..255]
		if (layer_index < 0) layer_index = 0;
		if (layer_index > 255) layer_index = 255;
		m_world.state.layer = layer_index;
		m_vars[TK_LAYER] = layer_index;

		while (m_world.stage.backbuffer.layers.size() <= m_world.state.layer)
		{
			m_world.stage.backbuffer.layers.emplace_back(m_world.stage.size);
		}
	}

	void Terminal::SetForeColor(Color color)
	{
		m_world.state.color = color;
		m_vars[TK_COLOR] = color;
	}

	void Terminal::SetBackColor(Color color)
	{
		m_world.state.bkcolor = color;
		m_vars[TK_BKCOLOR] = color;
	}

	void Terminal::SetComposition(int mode)
	{
		m_world.state.composition = mode;
		m_vars[TK_COMPOSITION] = mode;
	}

	void Terminal::SetFont(std::wstring name)
	{
		if (name.empty() || name == L"main")
		{
			m_world.state.font_offset = 0;
		}
		else
		{
			auto i = g_fonts.find(name);
			if (i != g_fonts.end())
			{
				m_world.state.font_offset = i->second * Tileset::kFontOffsetMultiplier;
			}
		}
	}

	void Terminal::PutInternal(int x, int y, int dx, int dy, char32_t code, Color* colors)
	{
		if (x < 0 || y < 0 || x >= m_world.stage.size.width || y >= m_world.stage.size.height) return;

		// Prepare tile if necessary.
		TileInfo* tile_info = nullptr;
		auto it = g_codespace.find(code);
		if (it != g_codespace.end())
			tile_info = it->second.get();
		else
			tile_info = GetTileInfo(code);

		// NOTE: layer must be already allocated by SetLayer
		int index = y*m_world.stage.size.width+x;
		Cell& cell = m_world.stage.backbuffer.layers[m_world.state.layer].cells[index];

		if (code != 0)
		{
			if (m_world.state.composition == TK_OFF)
			{
				cell.leafs.clear();
			}

			cell.leafs.emplace_back();
			Leaf& leaf = cell.leafs.back();

			// Character
			leaf.code = code;

			// Offset
			leaf.dx = dx;
			leaf.dy = dy;

			// Foreground colors
			if (colors)
			{
				for (int i=0; i<4; i++) leaf.color[i] = colors[i];
				leaf.flags |= Leaf::CornerColored;
			}
			else
			{
				leaf.color[0] = m_world.state.color;
			}

			// Background color
			if (m_world.state.layer == 0 && m_world.state.bkcolor)
			{
				for (int by = y; by < std::min(y+tile_info->spacing.height, m_world.stage.size.height); by++)
				{
					for (int bx = x; bx < std::min(x+tile_info->spacing.width, m_world.stage.size.width); bx++)
					{
						m_world.stage.backbuffer.background[by*m_world.stage.size.width+bx] = m_world.state.bkcolor;
					}
				}
			}
		}
		else
		{
			// Character code '0' means 'erase cell'
			cell.leafs.clear();
			if (m_world.state.layer == 0)
			{
				m_world.stage.backbuffer.background[index] = Color(); // Transparent color, no background
			}
		}
	}

	void Terminal::Put(int x, int y, int code)
	{
		PutExtended(x, y, 0, 0, code, nullptr);
	}

	void Terminal::PutExtended(int x, int y, int dx, int dy, int code, Color* corners)
	{
		if (m_options.terminal_encoding_affects_put)
		{
			code = m_encoding->Convert(code);
		}

		PutInternal(x, y, dx, dy, m_world.state.font_offset + code, corners);
	}

	int Terminal::Pick(int x, int y, int index)
	{
		if (x < 0 || y < 0 || x >= m_world.stage.size.width || y >= m_world.stage.size.height) return 0;

		int cell_index = y * m_world.stage.size.width + x;
		auto& cell = m_world.stage.backbuffer.layers[m_world.state.layer].cells[cell_index];
		wchar_t code = 0;
		if (index >= 0 && index < (int)cell.leafs.size())
			code = (int)(cell.leafs[index].code & Tileset::kCharOffsetMask);

		// Must take into account possible terminal.encoding codepage.
		int translated = m_encoding->Convert(code);
		return translated >= 0? translated: (int)code;
	}

	Color Terminal::PickForeColor(int x, int y, int index)
	{
		if (x < 0 || y < 0 || x >= m_world.stage.size.width || y >= m_world.stage.size.height) return Color();

		int cell_index = y * m_world.stage.size.width + x;
		auto& cell = m_world.stage.backbuffer.layers[m_world.state.layer].cells[cell_index];
		return (index >= 0 && index < (int)cell.leafs.size())? cell.leafs[index].color[0]: Color();
	}

	Color Terminal::PickBackColor(int x, int y)
	{
		if (x < 0 || y < 0 || x >= m_world.stage.size.width || y >= m_world.stage.size.height) return Color();

		int cell_index = y * m_world.stage.size.width + x;
		return m_world.stage.backbuffer.background[cell_index];
	}

	struct Line
	{
		struct Symbol
		{
			Symbol();
			Symbol(int code);
			Symbol(int code, Size spacing);
			int code;
			Size spacing;
		};

		Line();
		void UpdateSize();
		std::vector<Symbol> symbols;
		Size size;
	};

	Line::Symbol::Symbol():
		code(0)
	{ }

	Line::Symbol::Symbol(int code):
		code(code)
	{ }

	Line::Symbol::Symbol(int code, Size spacing):
		code(code),
		spacing(spacing)
	{ }

	Line::Line():
		size(0, 1)
	{ }

	void Line::UpdateSize()
	{
		for (auto& symbol: symbols)
		{
			if (symbol.code <= 0)
			{
				continue;
			}

			size.width += symbol.spacing.width;
			size.height = std::max(size.height, symbol.spacing.height);
		}
	}

	Size Terminal::Print(int x0, int y0, int w0, int h0, int align, std::wstring str, bool raw, bool measure_only)
	{
		char32_t font_offset = m_world.state.font_offset;
		bool combine = false;
		Point offset = Point(0, 0);
		Size wrap = Size{w0, h0};
		State original_state = m_world.state;

		int x, y, w;

		std::vector<std::function<void()>> tags;
		std::list<Line> lines;
		lines.emplace_back();

		auto GetTileSpacing = [&](char32_t code) -> Size
		{
			if (auto tile = GetTileInfo(code))
				return tile->spacing;

			return Size(1, 1);
		};

		auto AppendSymbol = [&](char32_t wcode)
		{
			char32_t code = font_offset + wcode;

			if (code == 0)
			{
				return;
			}

			if (combine)
			{
				tags.push_back([&, code]
				{
					if (w == -1) return;
					auto saved = m_world.state.composition;
					m_world.state.composition = TK_ON;
					PutInternal(w, y, offset.x, offset.y, code, nullptr);
					m_world.state.composition = saved;
				});
				lines.back().symbols.emplace_back(-(int)(tags.size()-1));
				combine = false;
			}
			else
			{
				lines.back().symbols.emplace_back((int)code, GetTileSpacing(code));
			}
		};

		for (size_t i = 0; i < str.length(); i++)
		{
			wchar_t c = str[i];

			if (c == L'[' && !raw && m_options.output_postformatting) // tag start
			{
				if (++i >= str.length()) // malformed
				{
					continue;
				}
				if (str[i] == L'[') // escaped left bracket
				{
					AppendSymbol(L'[');
					continue;
				}

				size_t closing_bracket_pos = str.find(L']', i);
				if (closing_bracket_pos == std::wstring::npos) // malformed
				{
					continue;
				}

				size_t params_pos = str.find(L'=', i);
				params_pos = std::min(closing_bracket_pos, (params_pos == std::wstring::npos)? str.length(): params_pos);

				std::wstring name = str.substr(i, params_pos-i);
				std::wstring params = (params_pos < closing_bracket_pos)? str.substr(params_pos+1, closing_bracket_pos-(params_pos+1)): std::wstring();
				char32_t arbitrary_code = 0;

				std::function<void()> tag;

				if ((name == L"color" || name == L"c") && !params.empty())
				{
					color_t color = Palette::Instance.Get(params);
					tag = [&, color]{m_world.state.color = color;};
				}
				else if (name == L"/color" || name == L"/c")
				{
					tag = [&]{m_world.state.color = original_state.color;};
				}
				else if ((name == L"bkcolor" || name == L"b") && !params.empty())
				{
					color_t color = Palette::Instance.Get(params);
					tag = [&, color]{m_world.state.bkcolor = color;};
				}
				else if (name == L"/bkcolor" || name == L"/b")
				{
					tag = [&]{m_world.state.bkcolor = original_state.bkcolor;};
				}
				else if (name == L"offset")
				{
					Point value = parse<Point>(params);
					tag = [&offset, value]{offset = value;};
				}
				else if (name == L"/offset")
				{
					tag = [&offset]{offset = Point(0, 0);};
				}
				else if (name == L"+")
				{
					combine = true;
				}
				else if (name == L"font")
				{
					auto i = g_fonts.find(params);
					font_offset = (i == g_fonts.end()? 0: i->second * 0x01000000);
				}
				else if (name == L"/font")
				{
					font_offset = 0;
				}
				else if (name == L"raw")
				{
					raw = true;
				}
				else if (try_parse(name, arbitrary_code))
				{
					AppendSymbol(arbitrary_code);
				}
				else
				{
					std::wstring subs;
					if (Config::Instance().TryGet(name, subs))
					{
						str.insert(closing_bracket_pos+1, subs);
						if (str.length() > m_world.stage.size.Area())
						{
							break; // Overflow, most likely it is an recursive expanding.
						}
					}
				}

				if (tag)
				{
					tags.push_back(std::move(tag));
					lines.back().symbols.emplace_back(-(int)(tags.size()-1));
				}

				i = closing_bracket_pos;
			}
			else if (c == L']' && !raw && m_options.output_postformatting)
			{
				if (++i >= str.length()) // malformed
				{
					continue;
				}
				else if (str[i] == L']') // escaped right bracket
				{
					AppendSymbol(L']');
				}
			}
			else if (c == L'\t')
			{
				for (int i = 0; i < m_options.output_tab_width; i++)
				{
					AppendSymbol(L' ');
				}
			}
			else if (c == L'\n') // forced line-break
			{
				lines.emplace_back();
				lines.back().size.height = GetTileSpacing(font_offset + L' ').height;
			}
			else if (c == L'\r')
			{
				// Ignore.
			}
			else
			{
				AppendSymbol(c);
			}
		}

		if (wrap.width > 0) // Auto-wrap the lines
		{
			for (auto i = lines.begin(); i != lines.end(); i++) // maybe, vector?
			{
				auto& line = *i;

				int length = 0, last_line_break = 0;
				for (size_t j = 0; j < line.symbols.size(); j++)
				{
					Line::Symbol& s = line.symbols[j];

					if (s.code <= 0) // tag reference
					{
						continue;
					}

					if (length + s.spacing.width > wrap.width) // cut off // FIXME: prove bounds correctness
					{
						if (last_line_break == 0)
						{
							// If there was no line-break characters in the line, cut the word in half.
							// Current symbol makes work overflow so it cannot be left on this line.
							last_line_break = j - 1;
						}

						int offset = last_line_break + 1;
						int leave = offset;

						if ((line.symbols[last_line_break].code & Tileset::kCharOffsetMask) == L' ')
						{
							leave -= 1;
						}

						auto copy = i;
						Line next;
						next.symbols = std::vector<Line::Symbol>(line.symbols.begin()+offset, line.symbols.end());
						lines.insert(++copy, next);
						line.symbols.resize(leave);

						break;
					}
					else
					{
						int relative_index = (s.code & Tileset::kCharOffsetMask);
						if (relative_index == (int)L' ' || relative_index == (int)L'-')
						{
							last_line_break = j;
						}
					}

					length += s.spacing.width;
				}
			}
		}

		int total_height = 0;
		int total_width = 0;
		for (auto& line: lines)
		{
			line.UpdateSize();
			total_height += line.size.height;
			total_width = std::max(total_width, line.size.width);
		}

		int horizontal_align = (align & 3);
		int vertical_align = (align & 12);

		if (!measure_only)
		{
			if ((vertical_align & TK_ALIGN_MIDDLE) == TK_ALIGN_MIDDLE)
			{
				y = y0 + std::ceil(wrap.height/2.0f - total_height/2.0f);
			}
			else if ((vertical_align & TK_ALIGN_BOTTOM) == TK_ALIGN_BOTTOM)
			{
				y = y0 + std::max(wrap.height, 1) - total_height;
			}
			else // TK_ALIGN_TOP or default
			{
				y = y0;
			}

			int cutoff_top = y0;
			int cutoff_bottom = cutoff_top + wrap.height-1;

			for (auto& line: lines)
			{
				int line_bottom = y + (line.size.height - 1);

				if (wrap.height == 0 || (y >= cutoff_top && y <= cutoff_bottom) || (line_bottom >= cutoff_top && line_bottom <= cutoff_bottom))
				{
					if ((horizontal_align & TK_ALIGN_CENTER) == TK_ALIGN_CENTER)
					{
						x = x0 + std::ceil(wrap.width/2.0f - (line.size.width-0)/2.0f);
					}
					else if ((horizontal_align & TK_ALIGN_RIGHT) == TK_ALIGN_RIGHT)
					{
						x = x0 + std::max(wrap.width, 1) - line.size.width;
					}
					else // TK_ALIGN_LEFT or default
					{
						x = x0;
					}

					w = -1;

					for (auto& s: line.symbols)
					{
						if (s.code > 0)
						{
							PutInternal(x, y, offset.x, offset.y, s.code, nullptr);
							w = x;
							x += s.spacing.width;
						}
						else
						{
							tags[-s.code]();
						}
					}
				}

				y += line.size.height;
			}

			m_world.state = original_state;
		}

		if (wrap.height)
			total_height = std::min(total_height, wrap.height);
		return Size{total_width, total_height};
	}

	bool Terminal::IsEventFiltered(int code)
	{
		return m_options.input_filter.empty() || m_options.input_filter.count(code);
	}

	bool Terminal::HasFilteredInput()
	{
		for (auto& e: m_input_queue)
		{
			if (IsEventFiltered(e.code))
				return true;
		}

		return false;
	}

	int Terminal::HasInput()
	{
		CHECK_THREAD("has_input", 0);

		m_window->PumpEvents();

		if (m_state != kVisible)
			return 1;

		return HasFilteredInput();
	}

	int Terminal::GetState(int code)
	{
		return (code >= 0 && code < (int)m_vars.size())? m_vars[code]: 0;
	}

	Event Terminal::ReadEvent(int timeout) // FIXME: more precise wait
	{
		if (m_state != kVisible)
			return {TK_CLOSE};

		auto started = std::chrono::system_clock::now();

		do
		{
			m_window->PumpEvents();

			if (HasFilteredInput())
			{
				while (!m_input_queue.empty())
				{
					Event e = m_input_queue.front();
					m_input_queue.pop_front();
					ConsumeEvent(e);
					if (IsEventFiltered(e.code))
						return e;
				}

				return 0;
			}
			else
			{
				//Delay(5);
				std::this_thread::sleep_for(std::chrono::milliseconds{5});
			}
		}
		while (std::chrono::system_clock::now() - started < std::chrono::milliseconds{timeout});

		return {TK_INPUT_NONE};
	}

	int Terminal::Read()
	{
		CHECK_THREAD("read", TK_CLOSE);

		return ReadEvent(std::numeric_limits<int>::max()).code;
	}

	int Terminal::Peek()
	{
		CHECK_THREAD("peek", TK_CLOSE);

		m_window->PumpEvents();

		if (m_state != kVisible)
		{
			return TK_CLOSE;
		}
		else if (HasFilteredInput())
		{
			for (auto& e: m_input_queue)
			{
				ConsumeEvent(e);
				if (IsEventFiltered(e.code))
					return e.code;
			}
		}
		else
		{
			return TK_INPUT_NONE;
		}
	}

	/**
	 * Reads whole string.
	 */
	int Terminal::ReadString(int x, int y, wchar_t* buffer, int max)
	{
		CHECK_THREAD("read_str", TK_INPUT_CANCELLED);

		std::vector<Cell> original;
		int composition_mode = m_world.state.composition;
		m_world.state.composition = TK_ON;

		if (buffer == nullptr || max <= 0)
		{
			LOG(Error, "Invalid buffer parameters were passed to string reading function");
			return 0;
		}

		if (x < 0 || x >= m_world.stage.size.width || y < 0 || y >= m_world.stage.size.height)
		{
			LOG(Error, "Invalid location parameters were passed to string reading function");
			return 0;
		}

		max = std::min(max, m_world.stage.size.width-x);
		for (int i=0; i<max; i++)
		{
			Layer& layer = m_world.stage.backbuffer.layers[m_world.state.layer];
			original.push_back(layer.cells[y*m_world.stage.size.width+x+i]);
		}

		// Garbage string protection
		for (int i=0, f=0; i<max+1; i++) if (f) buffer[i] = 0; else f = !buffer[i];
		buffer[max] = 0;
		int cursor = std::wcslen(buffer);

		auto put_buffer = [&](bool put_cursor)
		{
			Print(x, y, 0, 0, TK_ALIGN_DEFAULT, buffer, true, false);
			if (put_cursor && cursor < max) Put(x+cursor, y, m_options.input_cursor_symbol);
		};

		auto restore_scene = [&]()
		{
			for (int i = 0; i < max; i++)
			{
				Layer& layer = m_world.stage.backbuffer.layers[m_world.state.layer];
				layer.cells[y*m_world.stage.size.width+x+i] = original[i];
			}
		};

		int rc = 0;
		bool show_cursor = true;

		while (true)
		{
			restore_scene();
			put_buffer(show_cursor);
			Refresh();

			int blink_rate = m_options.input_cursor_blink_rate;
			auto event = ReadEvent(blink_rate? blink_rate: std::numeric_limits<int>::max());

			if (event.code == TK_INPUT_NONE)
			{
				// Timed out
				show_cursor = !show_cursor;
			}
			else if (event.code == TK_RETURN)
			{
				rc = wcslen(buffer);
				break;
			}
			else if (event.code == TK_ESCAPE || event.code == TK_CLOSE)
			{
				rc = TK_INPUT_CANCELLED;
				break;
			}
			else if (event.code == TK_BACKSPACE)
			{
				if (cursor > 0)
				{
					buffer[--cursor] = 0;
					show_cursor = true;
				}
			}
			else if (wchar_t ch = GetState(TK_WCHAR))
			{
				if (cursor < max)
				{
					buffer[cursor++] = ch;
					show_cursor = true;
				}
			}
		}

		// Restore
		restore_scene();
		m_world.state.composition = composition_mode;

		return rc;
	}

	void Terminal::Delay(int period)
	{
		CHECK_THREAD("delay", );

		auto until = std::chrono::system_clock::now() + std::chrono::milliseconds{period};
		std::chrono::system_clock::duration step = std::chrono::milliseconds{5};

		while (true)
		{
			int pumped = m_window->PumpEvents();
			auto left = until - std::chrono::system_clock::now();
			if (left <= std::chrono::system_clock::duration::zero())
				break;
			if (!pumped)
				std::this_thread::sleep_for(std::min(step, left));
		}
	}

	const Encoding<char>& Terminal::GetEncoding() const
	{
		return *m_encoding;
	}

	std::wstring Terminal::GetClipboard()
	{
		return m_window->GetClipboard();
	}

	void Terminal::ConfigureViewport()
	{
		Size viewport_size = m_window->GetActualSize();

		Size stage_size = m_world.stage.size * m_world.state.cellsize;
		m_stage_area = Rectangle(stage_size);
		m_stage_area_factor = SizeF(1, 1);

		if (viewport_size != stage_size)
		{
			if (m_vars[TK_FULLSCREEN])
			{
				// Stretch
				float viewport_ratio = viewport_size.width / (float)viewport_size.height;
				float stage_ratio = stage_size.width / (float)stage_size.height;

				if (viewport_ratio >= stage_ratio)
				{
					// Viewport is wider
					float factor = viewport_size.height / (float)stage_size.height;
					m_stage_area.height = viewport_size.height;
					m_stage_area.width = stage_size.width * factor;
					m_stage_area.left = (viewport_size.width - m_stage_area.width)/2;
				}
				else
				{
					// Stage is wider
					float factor = viewport_size.width / (float)stage_size.width;
					m_stage_area.width = viewport_size.width;
					m_stage_area.height = stage_size.height * factor;
					m_stage_area.top = (viewport_size.height - m_stage_area.height)/2;
				}
			}
			else
			{
				// Center
				float scale_factor = kScaleSteps[m_scale_step];
				m_stage_area.width *= scale_factor;
				m_stage_area.height *= scale_factor;
				m_stage_area.left += (viewport_size.width-m_stage_area.width)/2;
				m_stage_area.top += (viewport_size.height-m_stage_area.height)/2;
			}

			m_stage_area_factor = stage_size/m_stage_area.Size().As<float>();
		}

		glDisable(GL_DEPTH_TEST);
		glClearColor(0, 0, 0, 1);
		glViewport(0, 0, viewport_size.width, viewport_size.height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho
		(
			-m_stage_area.left * m_stage_area_factor.width,
			(viewport_size.width - m_stage_area.left) * m_stage_area_factor.width,
			(viewport_size.height - m_stage_area.top) * m_stage_area_factor.height,
			-m_stage_area.top * m_stage_area_factor.height,
			-1,
			+1
		);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		m_viewport_scissors = Rectangle
		(
			m_stage_area.left,
			viewport_size.height - m_stage_area.height - m_stage_area.top,
			m_stage_area.width,
			m_stage_area.height
		);

		m_viewport_scissors_enabled = viewport_size != stage_size;

		// ?..
		m_window->SetVSync(m_options.output_vsync);
	}

	void DrawTile(const Leaf& leaf, const TileInfo& tile, int x, int y, int w2, int h2)
	{
		// TODO: Think up of some optimization?
		// There are a lot of calculations done.

		int left, top;

		w2 *= tile.spacing.width;
		h2 *= tile.spacing.height;

		switch (tile.alignment)
		{
		case TileAlignment::Center:
		case TileAlignment::DeadCenter:
			left = x + tile.offset.x + w2 + leaf.dx;
			top = y + tile.offset.y + h2 + leaf.dy;
			break;
		case TileAlignment::TopRight:
			left = x + tile.offset.x + 2*w2 - tile.useful_space.width + leaf.dx;
			top = y + tile.offset.y + leaf.dy;
			break;
		case TileAlignment::BottomLeft:
			left = x + tile.offset.x + leaf.dx;
			top = y + tile.offset.y + 2*h2 - tile.useful_space.height + leaf.dy;
			break;
		case TileAlignment::BottomRight:
			left = x + tile.offset.x + 2*w2 - tile.useful_space.width + leaf.dx;
			top = y + tile.offset.y + 2*h2 - tile.useful_space.height + leaf.dy;
			break;
		case TileAlignment::TopLeft:
		default:
			// Same as TopLeft
			left = x + tile.offset.x + leaf.dx;
			top = y + tile.offset.y + leaf.dy;
			break;
		}

		int right = left + tile.useful_space.width;
		int bottom = top + tile.useful_space.height;

		if (leaf.flags & Leaf::CornerColored)
		{
			/*
			// Single-quad version (incorrect interpolation)
			// Top-left
			glColor4ub(leaf.color[0].r, leaf.color[0].g, leaf.color[0].b, leaf.color[0].a);
			glTexCoord2f(texture_coords.tu1, texture_coords.tv1);
			glVertex2i(left, top);

			// Bottom-left
			glColor4ub(leaf.color[1].r, leaf.color[1].g, leaf.color[1].b, leaf.color[1].a);
			glTexCoord2f(texture_coords.tu1, texture_coords.tv2);
			glVertex2i(left, bottom);

			// Bottom-right
			glColor4ub(leaf.color[2].r, leaf.color[2].g, leaf.color[2].b, leaf.color[2].a);
			glTexCoord2f(texture_coords.tu2, texture_coords.tv2);
			glVertex2i(right, bottom);

			// Top-right
			glColor4ub(leaf.color[3].r, leaf.color[3].g, leaf.color[3].b, leaf.color[3].a);
			glTexCoord2f(texture_coords.tu2, texture_coords.tv1);
			glVertex2i(right, top);
			/*/

			// 2-quad version
			// Center color
			int cr = (leaf.color[0].r + leaf.color[1].r + leaf.color[2].r + leaf.color[3].r)/4;
			int cg = (leaf.color[0].g + leaf.color[1].g + leaf.color[2].g + leaf.color[3].g)/4;
			int cb = (leaf.color[0].b + leaf.color[1].b + leaf.color[2].b + leaf.color[3].b)/4;
			int ca = (leaf.color[0].a + leaf.color[1].a + leaf.color[2].a + leaf.color[3].a)/4;
			// Center texture coords
			float cu = (tile.texture_coords.tu1 + tile.texture_coords.tu2)/2.0f;
			float cv = (tile.texture_coords.tv1 + tile.texture_coords.tv2)/2.0f;
			// Center coordinate
			float cx = (left + right)/2.0f;
			float cy = (top + bottom)/2.0f;

			// First quad
			// Top-left
			glColor4ub(leaf.color[0].r, leaf.color[0].g, leaf.color[0].b, leaf.color[0].a);
			glTexCoord2f(tile.texture_coords.tu1, tile.texture_coords.tv1);
			glVertex2i(left, top);
			// Bottom-left
			glColor4ub(leaf.color[1].r, leaf.color[1].g, leaf.color[1].b, leaf.color[1].a);
			glTexCoord2f(tile.texture_coords.tu1, tile.texture_coords.tv2);
			glVertex2i(left, bottom);
			// Center
			glColor4ub(cr, cg, cb, ca);
			glTexCoord2f(cu, cv);
			glVertex2i(cx, cy);
			// Top-right
			glColor4ub(leaf.color[3].r, leaf.color[3].g, leaf.color[3].b, leaf.color[3].a);
			glTexCoord2f(tile.texture_coords.tu2, tile.texture_coords.tv1);
			glVertex2i(right, top);

			// Second squad
			// Bottom-right
			glColor4ub(leaf.color[2].r, leaf.color[2].g, leaf.color[2].b, leaf.color[2].a);
			glTexCoord2f(tile.texture_coords.tu2, tile.texture_coords.tv2);
			glVertex2i(right, bottom);
			// Top-right
			glColor4ub(leaf.color[3].r, leaf.color[3].g, leaf.color[3].b, leaf.color[3].a);
			glTexCoord2f(tile.texture_coords.tu2, tile.texture_coords.tv1);
			glVertex2i(right, top);
			// Center
			glColor4ub(cr, cg, cb, ca);
			glTexCoord2f(cu, cv);
			glVertex2i(cx, cy);
			// Bottom-left
			glColor4ub(leaf.color[1].r, leaf.color[1].g, leaf.color[1].b, leaf.color[1].a);
			glTexCoord2f(tile.texture_coords.tu1, tile.texture_coords.tv2);
			glVertex2i(left, bottom);
			//*/
		}
		else
		{
			// Single-colored version
			glColor4ub(leaf.color[0].r, leaf.color[0].g, leaf.color[0].b, leaf.color[0].a);

			// Top-left
			glTexCoord2f(tile.texture_coords.tu1, tile.texture_coords.tv1);
			glVertex2i(left, top);

			// Bottom-left
			glTexCoord2f(tile.texture_coords.tu1, tile.texture_coords.tv2);
			glVertex2i(left, bottom);

			// Bottom-right
			glTexCoord2f(tile.texture_coords.tu2, tile.texture_coords.tv2);
			glVertex2i(right, bottom);

			// Top-right
			glTexCoord2f(tile.texture_coords.tu2, tile.texture_coords.tv1);
			glVertex2i(right, top);
		}
	}

	int Terminal::Redraw()
	{
		if (m_viewport_modified)
		{
			ConfigureViewport();
			m_viewport_modified = false;
		}

		// Clear must be done between scissoring test switch
		glDisable(GL_SCISSOR_TEST);
		glClear(GL_COLOR_BUFFER_BIT);

		if (m_viewport_scissors_enabled)
		{
			glEnable(GL_SCISSOR_TEST);
			auto& scissors = m_viewport_scissors;
			glScissor(scissors.left, scissors.top, scissors.width, scissors.height);
		}

		// Backgrounds
		Texture::Disable();
		glBegin(GL_QUADS);
		{
			int i = 0, left = 0, top = 0;
			int w = m_world.state.cellsize.width;
			int h = m_world.state.cellsize.height;
			for (int y=0; y<m_world.stage.size.height; y++)
			{
				for (int x=0; x<m_world.stage.size.width; x++)
				{
					Color& c = m_world.stage.frontbuffer.background[i];
					if (c.a > 0)
					{
						glColor4ub(c.r, c.g, c.b, c.a);
						glVertex2i(left+0, top+0);
						glVertex2i(left+0, top+h);
						glVertex2i(left+w, top+h);
						glVertex2i(left+w, top+0);
					}

					i += 1;
					left += w;
				}

				left = 0;
				top += h;
			}
		}
		glEnd();

		Texture::Enable();

		int w2 = m_world.state.half_cellsize.width;
		int h2 = m_world.state.half_cellsize.height;
		bool layer_scissors_applied = false;

		AtlasTexture* current_texture = nullptr;
		auto replacement_tile = GetTileInfo(kUnicodeReplacementCharacter);

		glBegin(GL_QUADS);
		glColor4f(1, 1, 1, 1);
		for (auto& layer: m_world.stage.frontbuffer.layers)
		{
			if (layer.crop.Area() > 0)
			{
				Rectangle scissors = layer.crop * m_world.state.cellsize / m_stage_area_factor;
				scissors.top = m_viewport_scissors.height - (scissors.top+scissors.height);
				scissors += m_viewport_scissors.Location();

				glEnd();
				glEnable(GL_SCISSOR_TEST);
				glScissor(scissors.left, scissors.top, scissors.width, scissors.height);
				glBegin(GL_QUADS);

				layer_scissors_applied = true;
			}

			int i = 0, left = 0, top = 0;

			for (int y=0; y<m_world.stage.size.height; y++)
			{
				for (int x=0; x<m_world.stage.size.width; x++)
				{
					for (auto& leaf: layer.cells[i].leafs)
					{
						auto i = g_codespace.find(leaf.code);
						auto tile = (i == g_codespace.end()? replacement_tile: i->second.get());

						if (tile->texture != current_texture)
						{
							glEnd();
							tile->texture->Bind();
							current_texture = tile->texture;
							glBegin(GL_QUADS);
						}

						DrawTile(leaf, *tile, left, top, w2, h2);
					}

					i += 1;
					left += m_world.state.cellsize.width;
				}

				left = 0;
				top += m_world.state.cellsize.height;
			}

			if (layer_scissors_applied)
			{
				glEnd();
				auto& scissors = m_viewport_scissors;
				glScissor(scissors.left, scissors.top, scissors.width, scissors.height);
				glBegin(GL_QUADS);
				layer_scissors_applied = false;
			}
		}
		glEnd();

		if (m_show_grid)
		{
			int width = m_world.stage.size.width * m_world.state.cellsize.width;
			int height = m_world.stage.size.height * m_world.state.cellsize.height;
			glColor4f(1, 1, 1, 0.5f);
			glDisable(GL_TEXTURE_2D);
			glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
			glBegin(GL_LINES);
			for (int i=0; i<=m_world.stage.size.width; i++)
			{
				int x = i*m_world.state.cellsize.width;
				//if (i == m_world.stage.size.width) x -= 1;
				glVertex2i(x, 0);
				glVertex2i(x, height);
			}
			for (int i=0; i<=m_world.stage.size.height; i++)
			{
				int y = i*m_world.state.cellsize.height;
				//if (i == m_world.stage.size.height) y -= 1;
				glVertex2i(0, y);
				glVertex2i(width, y);
			}
			glEnd();
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_TEXTURE_2D);
		}

		return 1;
	}

	void Terminal::PushEvent(Event event)
	{
		m_input_queue.push_back(event);
	}

	int Terminal::OnWindowEvent(Event event)
	{
		if (event.code == TK_REDRAW)
		{
			Render();
			return 0;
		}
		else if (event.code == TK_INVALIDATE)
		{
			m_viewport_modified = true;
			return 0;
		}
		// XXX: used to release keys on input focus gain (at least under Windows)
		else if (event.code == TK_ACTIVATED)
		{
			for (int i = TK_A; i <= TK_ALT; i++)
			{
				if (m_vars[i])
					PushEvent(Event(i|TK_KEY_RELEASED, {{i, 0}}));
			}

			// Clear Alt state which is commonly remain stuck, e. g. after Alt-Tab.
			m_alt_pressed = false;

			return 0;
		}
		else if (event.code == TK_RESIZED)
		{
			// Remove all unprocessed resize events as the latest one overrides them.
			for (auto i = m_input_queue.begin(); i != m_input_queue.end(); )
			{
				if (i->code == TK_RESIZED)
					i = m_input_queue.erase(i);
				else
					i++;
			}

			float scale_factor = kScaleSteps[m_scale_step];
			Size& cellsize = m_world.state.cellsize;
			Size size
			(
				std::floor(event[TK_WIDTH]/scale_factor/cellsize.width),
				std::floor(event[TK_HEIGHT]/scale_factor/cellsize.height)
			);

			if (size == m_world.stage.size)
			{
				// This event do not changes stage size, ignore.
				return 0;
			}
			else
			{
				// Translate pixels to cells.
				event[TK_WIDTH] = size.width;
				event[TK_HEIGHT] = size.height;
			}
		}
		else if (event.code == TK_MOUSE_MOVE)
		{
			int& pixel_x = event[TK_MOUSE_PIXEL_X];
			int& pixel_y = event[TK_MOUSE_PIXEL_Y];

			// Shift coordinates relative to stage area
			pixel_x -= m_stage_area.left;
			pixel_y -= m_stage_area.top;

			// Scale coordinates back to virtual pixels.
			pixel_x *= m_stage_area_factor.width;
			pixel_y *= m_stage_area_factor.height;

			// Cell location of mouse pointer
			Size& cellsize = m_world.state.cellsize;
			Point location(pixel_x / cellsize.width, pixel_y / cellsize.height);
			location = Rectangle(m_world.stage.size).Clamp(location);
			event[TK_MOUSE_X] = location.x;
			event[TK_MOUSE_Y] = location.y;

			// If application do not read events fast enough, do not flood it with mouse moves.
			if (!m_input_queue.empty() && m_input_queue.back().code == TK_MOUSE_MOVE)
			{
				// Replace the last, yet unread event with the most recent one.
				m_input_queue.back() = event;
				return 0;
			}

			// Ignore mouse movement events that do not change coarse cursor location.
			if (!m_options.input_precise_mouse)
			{
				Point last_location{m_vars[TK_MOUSE_X], m_vars[TK_MOUSE_Y]};

				// Search for last mouse movement event in the queue.
				for (auto i = m_input_queue.rbegin(); i != m_input_queue.rend(); i++)
				{
					if (i->code == TK_MOUSE_MOVE)
					{
						last_location = Point{(*i)[TK_MOUSE_X], (*i)[TK_MOUSE_Y]};
						break;
					}
				}

				if (location == last_location)
				{
					return 0;
				}
			}
		}
		else if ((event.code & 0xFF) == TK_ALT && m_options.input_alt_functions)
		{
			m_alt_pressed = event.properties[TK_ALT];
		}

		if (m_options.input_alt_functions && m_alt_pressed)
		{
			/*if (event.code == TK_A)
			{
				// Alt+A: dump atlas textures to disk.
				m_world.tiles.atlas.Dump();
				return 0;
			}
			else*/ if (event.code == TK_G)
			{
				// Alt+G: toggle grid
				m_show_grid = !m_show_grid;
				Render();
				return 0;
			}
			else if (event.code == TK_RETURN)
			{
				// Alt+ENTER: toggle fullscreen.
				m_vars[TK_FULLSCREEN] = !m_vars[TK_FULLSCREEN];
				m_options.window_fullscreen = m_vars[TK_FULLSCREEN];
				m_window->SetFullscreen(m_options.window_fullscreen);
				return 0;
			}
			else if (event.code == TK_MINUS || event.code == TK_EQUALS || event.code == TK_0 ||
			         event.code == TK_KP_MINUS || event.code == TK_KP_PLUS || event.code == TK_KP_0)
			{
				if (m_vars[TK_FULLSCREEN])
				{
					// No scaling in fullscreen mode (does not make sense anyway).
					return 0;
				}

				// Alt+(plus/minus/zero): adjust user window scaling.
				if ((event.code == TK_MINUS || event.code == TK_KP_MINUS) && m_scale_step > 0)
				{
					m_scale_step -= 1;
				}
				else if ((event.code == TK_EQUALS || event.code == TK_KP_PLUS) && m_scale_step < kScaleSteps.size()-1)
				{
					m_scale_step += 1;
				}
				else if ((event.code == TK_0 || event.code == TK_KP_0) && m_scale_step != 1)
				{
					m_scale_step = 1;
				}

				float scale = kScaleSteps[m_scale_step];

				if (m_options.window_resizeable || m_options.window_client_size.Area() == 0)
				{
					// Resizeable window always snaps to cell borders.
					m_window->SetSizeHints(m_world.state.cellsize * scale, m_options.window_minimum_size);
					m_window->SetClientSize(m_world.state.cellsize * m_world.stage.size * scale);
				}
				else
				{
					// Overriden client-size is scaled with everything else.
					m_window->SetClientSize(m_options.window_client_size * scale);
				}

				return 0;
			}
		}

		PushEvent(std::move(event));

		return 0;
	}

	void Terminal::ConsumeEvent(Event& event)
	{
		if (event.code == TK_RESIZED)
		{
			if (m_options.window_resizeable)
			{
				// Stage size changed, must reallocate and reconstruct scene
				m_options.window_size = Size(event[TK_WIDTH], event[TK_HEIGHT]);
				m_world.stage.Resize(m_options.window_size);

				// User resize cancels client-size
				// This one handles client-size set after resizeable.
				m_options.window_client_size = Size();

				// Client size changed, must redraw
				m_viewport_modified = true;
			}
			else
			{
				return;
			}
		}
		else if (event.code >= TK_KP_DIVIDE && event.code <= TK_KP_PERIOD)
		{
			// Keypad is translated to characters here because platform-specific translation is
			// * much more complex (e. g. WM_CHAR vs WM_KEYDOWN)
			// * not always possible (subject to NumLock and terminal ignores NumLock by design)
			static std::map<int, wchar_t> keypad_char_mapping =
			{
				{TK_KP_DIVIDE, L'/'}, {TK_KP_MULTIPLY, L'*'}, {TK_KP_MINUS, L'-'}, {TK_KP_PLUS, L'+'},
				{TK_KP_ENTER, 0},
				{TK_KP_1, L'1'}, {TK_KP_2, L'2'}, {TK_KP_3, L'3'},
				{TK_KP_4, L'4'}, {TK_KP_5, L'5'}, {TK_KP_6, L'6'},
				{TK_KP_7, L'7'}, {TK_KP_8, L'8'}, {TK_KP_9, L'9'},
				{TK_KP_0, L'0'}, {TK_KP_PERIOD, L'.'}
			};

			event.properties[TK_WCHAR] = keypad_char_mapping[event.code];
		}

		if (!event.properties.count(TK_WCHAR))
		{
			// Clear CHAR/WCHAR states if event does not produce any characters
			m_vars[TK_CHAR] = m_vars[TK_WCHAR] = 0;
		}
		else
		{
			int code = m_encoding->Convert((wchar_t)event.properties[TK_WCHAR]);

			if (code < 0 || (m_encoding->GetName() == L"utf-8" && code > 127))
			{
				// Use ASCII replacement character for codes not mapped to ANSI
				code = 0x1A;
			}

			event.properties[TK_CHAR] = code;
		}

		for (auto& slot: event.properties)
		{
			if (slot.first >= 0 && slot.first < m_vars.size())
			{
				m_vars[slot.first] = slot.second;
			}
		}

		m_vars[TK_EVENT] = event.code;
	}

	void Terminal::Render()
	{
		Redraw();
		m_window->SwapBuffers();
	}
}
