/*
 * Terminal.cpp
 *
 *  Created on: Sep 18, 2013
 *      Author: Cfyz
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

#include <iostream>

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
	static std::vector<float> kScaleSteps =
	{
		0.75f, 1.0f, 1.25f, 1.5f, 2.0f, 3.0f
	};

	static int kScaleDefault = 1;

	Terminal::Terminal():
		m_state{kHidden},
		m_vars{},
		m_show_grid{false},
		m_viewport_modified{false},
		m_scale_step(kScaleDefault)
	{
		// Reset logger (this is terrible)
		g_logger = std::unique_ptr<Log>(new Log());

		// Try to create window
		m_window = Window::Create();
		m_window->SetEventHandler(std::bind(&Terminal::OnWindowEvent, this, std::placeholders::_1));

		// Default parameters
		SetOptionsInternal(L"window: size=80x25, icon=default; font: default; terminal.encoding=utf8");
	}

	Terminal::~Terminal()
	{
		m_window->Stop();
		m_window.reset(); // TODO: is it needed?
	}

	int Terminal::SetOptions(const std::wstring& value)
	{
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

	void Terminal::ApplyTilesets(std::map<uint16_t, std::unique_ptr<Tileset>>& new_tilesets)
	{
		for (auto& i: new_tilesets)
		{
			auto j = m_world.tilesets.find(i.first);

			if (i.second)
			{
				// Add/update tileset
				if (j == m_world.tilesets.end())
				{
					// New tileset: add to registry and save
					m_world.tilesets[i.first] = std::move(i.second);
					m_world.tilesets[i.first]->Save();
					LOG(Debug, "Saved new tileset for base code " << i.first);
				}
				else
				{
					// Update an already existing one
					if (typeid(*i.second) == typeid(*j->second))
					{
						// Same type, reload
						j->second->Reload(std::move(*i.second));
						LOG(Debug, "Reloaded the tileset with base code " << i.first);
					}
					else
					{
						// Different types, replace
						j->second->Remove();
						LOG(Debug, "Unloaded old tileset for base code " << i.first);

						j->second = std::move(i.second);
						j->second->Save();
						LOG(Debug, "Saved new tileset for base code " << i.first);
					}
				}
			}
			else
			{
				// Empty pointer, remove tileset
				if (i.first > 0)
				{
					if (j != m_world.tilesets.end())
					{
						// Remove the tileset
						m_world.tilesets[i.first]->Remove();
						m_world.tilesets.erase(i.first);
						LOG(Debug, "Unloaded the tileset for base code " << i.first);
					}
				}
				else
				{
					LOG(Warning, "Tileset with base code 0 cannot be unloaded");
				}
			}
		}
	}

	void Terminal::UpdateDynamicTileset(Size size)
	{
		auto& tileset = m_world.tilesets[kUnicodeReplacementCharacter];
		if (tileset) tileset->Remove();

		OptionGroup options;
		options.name = L"0xFFFF";
		options.attributes[L"name"] = L"dynamic";
		options.attributes[L"size"] = to_string<wchar_t>(size);

		tileset = Tileset::Create(m_world.tiles, options);
		tileset->Save();
	}

	void Terminal::SetOptionsInternal(const std::wstring& value)
	{
		std::lock_guard<std::mutex> guard(m_lock);

		auto groups = ParseOptions(value);
		Options updated = m_options;
		std::map<uint16_t, std::unique_ptr<Tileset>> new_tilesets;

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
			else
			{
				uint16_t base_code = 0; // Basic font base_code is 0
				if (group.name == L"font" || try_parse(group.name, base_code))
				{
					new_tilesets[base_code] = Tileset::Create(m_world.tiles, group);
					LOG(Debug, "Successfully loaded a tileset for base code " << base_code);
				}
			}
		}

		if (updated.output_vsync != m_options.output_vsync)
		{
			m_window->SetVSync(updated.output_vsync);
		}

		// All options and parameters must be validated, may try to apply them
		if (!new_tilesets.empty())
		{
			ApplyTilesets(new_tilesets);
		}

		// Primary sanity check: if there is no base font, lots of things are gonna fail
		if (!m_world.tilesets.count(0))
		{
			throw std::runtime_error("No base font has been configured");
		}

		// Such implementation is awful. Should use some global (external library?) instance.
		if (updated.log_filename != m_options.log_filename) g_logger->SetFile(updated.log_filename);
		if (updated.log_level != m_options.log_level) g_logger->SetLevel(updated.log_level);
		if (updated.log_mode != m_options.log_mode) g_logger->SetMode(updated.log_mode);

		if (updated.terminal_encoding != m_options.terminal_encoding)
		{
			m_encoding = GetUnibyteEncoding(updated.terminal_encoding);
		}

		if (updated.input_mouse_cursor != m_options.input_mouse_cursor)
		{
			m_window->SetCursorVisibility(updated.input_mouse_cursor);
		}

		// Apply on per-option basis
		bool viewport_size_changed = false;

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
			m_window->SetResizeable(updated.window_resizeable);
		}

		if (updated.window_resizeable && m_scale_step != kScaleDefault)
		{
			// No scaling in resizeable mode, at least not yet.
			m_scale_step = kScaleDefault;
			viewport_size_changed = true;
		}

		// If the size of cell has changed -OR- new basic tileset has been specified
		if (updated.window_cellsize != m_options.window_cellsize || new_tilesets.count(0))
		{
			// Refresh stage.state.cellsize
			m_world.state.cellsize = updated.window_cellsize;

			// If specified cellsize is nil, fall back on base font bounding box
			if (!m_world.state.cellsize.Area())
			{
				// NOTE: by now, tileset container MUST have 0th tileset since
				// one is added in ctor and cannot be fully removed afterwards.
				m_world.state.cellsize = m_world.tilesets[0]->GetBoundingBoxSize();
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
			Size viewport_size = m_world.stage.size * m_world.state.cellsize * scale_factor;
			m_vars[TK_CLIENT_WIDTH] = viewport_size.width;
			m_vars[TK_CLIENT_HEIGHT] = viewport_size.height;
			m_window->SetSizeHints(m_world.state.cellsize*scale_factor, updated.window_minimum_size);
			m_window->SetClientSize(viewport_size);
			m_viewport_modified = true;
		}

		if (updated.window_toggle_fullscreen)
		{
			m_window->ToggleFullscreen();
			updated.window_toggle_fullscreen = false;
		}

		// Do not touch input lock. Input handlers should just take main lock if necessary.
		/*
		// Briefly grab input lock so that input routines do not contend for m_options
		std::lock_guard<std::mutex> input_guard(m_input_lock);
		//*/

		m_options = updated;
	}

	void Terminal::ValidateTerminalOptions(OptionGroup& group, Options& options)
	{
		// Possible options: encoding

		if (group.attributes.count(L"encoding"))
		{
			options.terminal_encoding = group.attributes[L"encoding"];
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

			if (value.width <= 0 || value.width >= 256 || value.height <= 0 || value.height >= 256)
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

		if (group.attributes.count(L"title"))
		{
			options.window_title = group.attributes[L"title"];
		}

		if (group.attributes.count(L"icon"))
		{
			if (!m_window->ValidateIcon(group.attributes[L"icon"]))
			{
				throw std::runtime_error("window.icon cannot be loaded");
			}

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
			bool desired_state = false;

			if (!try_parse(group.attributes[L"fullscreen"], desired_state))
			{
				throw std::runtime_error("window.fullscreen value cannot be parsed");
			}

			if (desired_state != m_window->IsFullscreen())
			{
				options.window_toggle_fullscreen = true;
			}
		}
	}

	void Terminal::ValidateInputOptions(OptionGroup& group, Options& options)
	{
		// Possible options: nonblocking, events, precise_mouse, sticky_close, cursor_symbol, cursor_blink_rate

		if (group.attributes.count(L"precise-mouse") && !try_parse(group.attributes[L"precise-mouse"], options.input_precise_mouse))
		{
			throw std::runtime_error("input.precise-mouse cannot be parsed");
		}

		if (group.attributes.count(L"sticky-close") && !try_parse(group.attributes[L"sticky-close"], options.input_sticky_close))
		{
			throw std::runtime_error("input.sticky-close cannot be parsed");
		}

		if (group.attributes.count(L"keyboard") && !try_parse(group.attributes[L"keyboard"], options.input_keyboard))
		{
			throw std::runtime_error("input.keyboard cannot be parsed");
		}

		if (group.attributes.count(L"mouse") && !try_parse(group.attributes[L"mouse"], options.input_mouse))
		{
			throw std::runtime_error("input.mouse cannot be parsed");
		}

		if (group.attributes.count(L"cursor-symbol") && !try_parse(group.attributes[L"cursor-symbol"], options.input_cursor_symbol))
		{
			throw std::runtime_error("input.cursor-symbol cannot be parsed");
		}

		if (group.attributes.count(L"cursor-blink-rate") && !try_parse(group.attributes[L"cursor-blink-rate"], options.input_cursor_blink_rate))
		{
			throw std::runtime_error("input.cursor-blink-rate cannot be parsed");
		}

		if (options.input_cursor_blink_rate <= 0) options.input_cursor_blink_rate = 1;

		if (group.attributes.count(L"mouse-cursor") && !try_parse(group.attributes[L"mouse-cursor"], options.input_mouse_cursor))
		{
			throw std::runtime_error("input.mouse-cursor cannot be parsed");
		}
	}

	void Terminal::ValidateOutputOptions(OptionGroup& group, Options& options)
	{
		// Possible options: postformatting, vsync

		if (group.attributes.count(L"postformatting") && !try_parse(group.attributes[L"postformatting"], options.output_postformatting))
		{
			throw std::runtime_error("output.postformatting cannot be parsed");
		}

		if (group.attributes.count(L"vsync") && !try_parse(group.attributes[L"vsync"], options.output_vsync))
		{
			throw std::runtime_error("output.vsync cannot be parsed");
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
		if (m_state == kHidden)
		{
			m_window->Show();
			m_state = kVisible;
		}

		if (m_state != kVisible) return;

		// Synchronously copy backbuffer to frontbuffer
		{
			std::lock_guard<std::mutex> guard(m_lock);
			m_world.stage.frontbuffer = m_world.stage.backbuffer;
		}

		m_window->Invoke([&]()
		{
			Redraw(false);
			m_window->SwapBuffers();
		});
	}
#endif

	void Terminal::Clear()
	{
		if (m_world.stage.backbuffer.background.size() != m_world.stage.size.Area())
		{
			LOG(Trace, "World resize");
			std::lock_guard<std::mutex> guard(m_lock);
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

	void Terminal::PutInternal(int x, int y, int dx, int dy, wchar_t code, Color* colors)
	{
		if (x < 0 || y < 0 || x >= m_world.stage.size.width || y >= m_world.stage.size.height) return;

		uint16_t u16code = (uint16_t)code;
		if (m_world.tiles.slots.find(u16code) == m_world.tiles.slots.end())
		{
			m_fresh_codes.push_back(u16code);
		}

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
			leaf.code = u16code;

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
				m_world.stage.backbuffer.background[index] = m_world.state.bkcolor;
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

	void Terminal::Put(int x, int y, wchar_t code)
	{
		PutInternal(x, y, 0, 0, code, nullptr);
	}

	void Terminal::PutExtended(int x, int y, int dx, int dy, wchar_t code, Color* corners)
	{
		PutInternal(x, y, dx, dy, code, corners);
	}

	struct Alignment
	{
		Alignment();

		enum // FIXME: enum class
		{
			Left,
			Center,
			Right,
			Top,
			Bottom
		}
		horisontal, vertical;
	};

	Alignment::Alignment():
		horisontal(Left),
		vertical(Top)
	{ }

	bool try_parse(const std::wstring& s, Alignment& out)
	{
		size_t hyphen_pos = s.find(L'-');

		// FIXME: rewrite this mess
		std::wstring vert = hyphen_pos != std::wstring::npos? s.substr(0, hyphen_pos): std::wstring();
		std::wstring hor = hyphen_pos != std::wstring::npos? (hyphen_pos < s.length()-1? (s.substr(hyphen_pos+1)): std::wstring()): s;

		Alignment result;

		if (vert == L"center")
		{
			result.vertical = Alignment::Center;
		}
		else if (vert == L"bottom")
		{
			result.vertical = Alignment::Bottom;
		}

		if (hor == L"center")
		{
			result.horisontal = Alignment::Center;
		}
		else if (hor == L"right")
		{
			result.horisontal = Alignment::Right;
		}

		out = result;
		return true;
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

	void Line::UpdateSize()
	{
		size = Size();
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

	int Terminal::Print(int x0, int y0, const std::wstring& str, bool measure_only)
	{
		uint16_t base = 0;
		const Encoding<char>* codepage = nullptr;
		bool combine = false;
		Point offset = Point(0, 0);
		Size wrap = Size(0, 0);
		Alignment alignment;
		bool raw = false;
		Color original_fore = m_world.state.color;
		Color original_back = m_world.state.bkcolor;

		int x, y, w;

		std::vector<std::function<void()>> tags;
		std::list<Line> lines;
		lines.emplace_back();

		auto GetTileSpacing = [&](wchar_t code) -> Size // TODO: GetResponsibleTileset?
		{
			for (auto i = m_world.tilesets.rbegin(); ; i++)
			{
				if (i->first <= code)
				{
					return i->second->GetSpacing();
				}
			}

			return Size(1, 1);
		};

		auto GetTileRelativeIndex = [&](wchar_t code) -> int
		{
			for (auto i = m_world.tilesets.rbegin(); ; i++)
			{
				if (i->first <= code)
				{
					return code - i->first;
				}
			}

			return 0;
		};

		auto AppendSymbol = [&](wchar_t code)
		{
			if (code == 0)
			{
				return;
			}

			if (codepage)
			{
				code = codepage->Convert(code);
			}

			if (code > 0)
			{
				code += base;
			}
			else
			{
				code = kUnicodeReplacementCharacter;
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

				if (!measure_only)
				{
					size_t params_pos = str.find(L'=', i);
					params_pos = std::min(closing_bracket_pos, (params_pos == std::wstring::npos)? str.length(): params_pos);

					std::wstring name = str.substr(i, params_pos-i);
					std::wstring params = (params_pos < closing_bracket_pos)? str.substr(params_pos+1, closing_bracket_pos-(params_pos+1)): std::wstring();
					uint16_t arbitrary_code = 0;

					std::function<void()> tag;

					if ((name == L"color" || name == L"c") && !params.empty())
					{
						color_t color = Palette::Instance[params];
						tag = [&, color]{m_world.state.color = color;};
					}
					else if (name == L"/color" || name == L"/c")
					{
						tag = [&]{m_world.state.color = original_fore;};
					}
					if ((name == L"bkcolor" || name == L"b") && !params.empty())
					{
						color_t color = Palette::Instance[params];
						tag = [&, color]{m_world.state.bkcolor = color;};
					}
					else if (name == L"/bkcolor" || name == L"/b")
					{
						tag = [&]{m_world.state.bkcolor = original_back;};
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
					else if (name == L"font" || name == L"base")
					{
						size_t colon_pos = params.find(L':');
						std::wstring codepage_name;
						if (colon_pos != std::wstring::npos && colon_pos > 0 && colon_pos < params.length()-1)
						{
							codepage_name = params.substr(colon_pos+1);
							params = params.substr(0, colon_pos);
						}

						if (!try_parse(params, base))
						{
							base = 0;
							codepage = nullptr;
							continue;
						}

						if (codepage_name.empty())
						{
							// Use tileset codepage
							auto i = m_world.tilesets.find(base);
							if (i != m_world.tilesets.end())
							{
								codepage = i->second->GetCodepage();
							}
							else
							{
								codepage = nullptr;
							}
						}
						else
						{
							auto cached = m_codepage_cache.find(codepage_name);
							if (cached != m_codepage_cache.end())
							{
								codepage = cached->second.get();
							}
							else
							{
								if (auto p = GetUnibyteEncoding(codepage_name))
								{
									codepage = p.get();
									m_codepage_cache[codepage_name] = std::move(p);
								}
								else
								{
									codepage = nullptr;
								}
							}
						}
					}
					else if (name == L"/font" || name == L"/base")
					{
						base = 0;
						codepage = nullptr;
					}
					else if (name == L"wrap" || name == L"bbox")
					{
						wrap = parse<Size>(params);
					}
					else if (name == L"align" || name == L"a")
					{
						alignment = parse<Alignment>(params);
					}
					else if (name == L"raw")
					{
						raw = true;
					}
					else if (try_parse(name, arbitrary_code))
					{
						AppendSymbol(arbitrary_code);
					}

					if (tag)
					{
						tags.push_back(std::move(tag));
						lines.back().symbols.emplace_back(-(int)(tags.size()-1));
					}
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
			else if (c == L'\n') // forced line-break
			{
				lines.emplace_back();
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
							last_line_break = j - 1;
						}

						int offset = last_line_break + 1;
						int leave = offset;

						if (GetTileRelativeIndex(line.symbols[last_line_break].code) == (int)L' ')
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
						int relative_index = GetTileRelativeIndex(s.code);
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

		if (!measure_only)
		{
			int cutoff_top, cutoff_bottom;

			switch (alignment.vertical)
			{
			case Alignment::Bottom:
				y = y0 - (total_height-1);
				cutoff_top = y0 - (wrap.height-1);
				cutoff_bottom = y0;
				break;
			case Alignment::Center:
				y = y0 - (int)std::floor((total_height-1)/2.0f);
				cutoff_top = y0 - (int)std::floor((wrap.height-1)/2.0f);
				cutoff_bottom = y0 + wrap.height;
				break;
			default: // Top
				y = y0;
				cutoff_top = y0;
				cutoff_bottom = y0 + (wrap.height-1);
				break;
			}

			for (auto& line: lines)
			{
				int line_bottom = y + line.size.height;

				if (wrap.height == 0 || (y >= cutoff_top && y <= cutoff_bottom) || (line_bottom >= cutoff_top && line_bottom <= cutoff_bottom))
				{
					switch (alignment.horisontal)
					{
					case Alignment::Right:
						x = x0 - (line.size.width - 1);
						break;
					case Alignment::Center:
						x = x0 - (int)std::floor(line.size.width/2.0f);
						break;
					default: // Left
						x = x0;
						break;
					}

					w = -1;

					for (auto& s: line.symbols)
					{
						if (s.code > 0)
						{
							PutInternal(x, y, offset.x, offset.y, (wchar_t)s.code, nullptr);
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

			m_world.state.color = original_fore;
			m_world.state.bkcolor = original_back;
		}

		return wrap.Area()? total_height: total_width;
	}

	/*
	int Terminal::Print(int x0, int y0, const std::wstring& s)
	{
		if (!s.empty() && s[0] != '^')
		{
			return ProcessPrint(x0, y0, s, false);
		}

		int x = x0, y = y0;
		int printed = 0;
		uint16_t base = 0;
		Encoding<char>* codepage = nullptr;
		bool combine = false;
		Size spacing{1, 1};
		Point offset{0, 0};

		Color original_color = m_world.state.color;
		Color original_bkcolor = m_world.state.bkcolor;

		Size size = m_world.stage.size;

		const auto put_and_increment = [&](int code)
		{
			// Convert from unicode to tileset codepage
			if (codepage)
			{
				code = codepage->Convert((wchar_t)code);
			}

			// Offset tile index
			if (code >= 0)
			{
				code += base;
			}
			else
			{
				code = kUnicodeReplacementCharacter;
			}

			if (combine)
			{
				if (x >= x0+spacing.width)
				{
					int composition = m_world.state.composition;
					m_world.state.composition = TK_ON;
					x -= spacing.width;
					PutInternal(x, y, offset.x, offset.y, code, nullptr);
					m_world.state.composition = composition;
				}
				combine = false;
			}
			else
			{
				PutInternal(x, y, offset.x, offset.y, code, nullptr);
			}

			x += spacing.width;
			printed += 1;
		};

		const auto apply_tag = [&](const std::wstring& s, size_t begin, size_t end)
		{
			if (s[begin+1] == L'/')
			{
				// Cancel tag: [/name]

				std::wstring name = s.substr(begin+2, end-(begin+2));

				if (name == L"color")
				{
					m_world.state.color = original_color;
				}
				else if (name == L"bkcolor")
				{
					m_world.state.bkcolor = original_bkcolor;
				}
				else if (name == L"base")
				{
					base = 0;
					codepage = nullptr;
				}
				else if (name == L"spacing")
				{
					spacing = Size{1, 1};
				}
				else if (name == L"offset")
				{
					offset = Point{0, 0};
				}
			}
			else
			{
				// Set tag: [name] or [name=value]
				std::wstring name, value;

				size_t n_equals = s.find(L'=', begin);
				if (n_equals == std::wstring::npos || n_equals > end-1)
				{
					name = s.substr(begin+1, end-(begin+1));
				}
				else
				{
					name = s.substr(begin+1, n_equals-(begin+1));
					value = s.substr(n_equals+1, end-(n_equals+1));
				}

				if (name.length() == 0) return;

				if (name == L"color")
				{
					m_world.state.color = Palette::Instance[value];
				}
				else if (name == L"bkcolor")
				{
					m_world.state.bkcolor = Palette::Instance[value];
				}
				else if (name == L"base")
				{
					// Optional codepage: "U+E100:1251"
					size_t n_colon = value.find(L":");
					if (n_colon != std::wstring::npos && n_colon > 0 && n_colon < value.length()-1)
					{
						std::wstring codepage_name = value.substr(n_colon+1);
						value = value.substr(0, n_colon);

						auto cached = m_codepage_cache.find(codepage_name);
						if (cached != m_codepage_cache.end())
						{
							codepage = cached->second.get();
						}
						else
						{
							if (auto p = GetUnibyteEncoding(codepage_name))
							{
								codepage = p.get();
								m_codepage_cache[codepage_name] = std::move(p);
							}
						}
					}

					if (!try_parse(value, base))
					{
						base = 0;
						codepage = nullptr;
					}
				}
				else if (name == L"spacing")
				{
					if (value.find(L"x") != std::wstring::npos)
					{
						if (!try_parse(value, spacing)) spacing = Size{1, 1};
					}
					else
					{
						if (!try_parse(value, spacing.width)) spacing = Size{1, 1};
					}

					if (spacing.width <= 0 || spacing.height <= 0)
					{
						spacing = Size{1, 1};
					}
				}
				else if (name == L"offset")
				{
					if (!try_parse(value, offset)) offset = Point{0, 0};
				}
				else if (name[0] == L'+')
				{
					combine = true;
				}
				else if (name[0] == L'u' || name[0] == L'U')
				{
					if (name.length() > 2)
					{
						std::wstringstream stream;
						stream << std::hex;
						stream << name.substr(2);
						uint16_t value = 0;
						stream >> value;
						put_and_increment(value);
					}
				}
			}
		};

		for (size_t i = 0; i < s.length(); i++)
		{
			wchar_t c = s[i];

			if (c == L'[' && m_options.output_postformatting)
			{
				if (i == s.length()-1) break; // Malformed postformatting tag

				if (s[i+1] == L'[')
				{
					// Escaped '['
					i += 1;
					put_and_increment(s[i]);
				}
				else
				{
					// Start of a postformatting tag
					size_t closing = s.find(L']', i);
					if (closing == std::wstring::npos) break; // Malformed, no closing ']'
					apply_tag(s, i, closing);
					i = closing;
				}
			}
			else if (c == L']' && m_options.output_postformatting)
			{
				// This MUST be an escaped ']' because regular closing postformatting ']' will be
				// consumed while parsing that tag

				if (i == s.length()-1) break; // Malformed

				i += 1;
				put_and_increment(s[i]);
			}
			else if (c == '\n')
			{
				x = x0;
				y += spacing.height;
			}
			else
			{
				put_and_increment(c);
			}
		}

		// Revert state
		m_world.state.color = original_color;
		m_world.state.bkcolor = original_bkcolor;

		return printed;
	}
	//*/

	bool Terminal::HasInputInternalUnlocked()
	{
		return !m_input_queue.empty();
	}

	int Terminal::HasInput()
	{
		std::lock_guard<std::mutex> guard(m_input_lock);
		if (m_state == kClosed || m_vars[TK_CLOSE]) return 1;
		return HasInputInternalUnlocked();
	}

	int Terminal::GetState(int code)
	{
		std::lock_guard<std::mutex> guard(m_input_lock);
		return (code >= 0 && code < (int)m_vars.size())? m_vars[code]: 0;
	}

	Event Terminal::ReadEvent(int timeout)
	{
		std::unique_lock<std::mutex> lock(m_input_lock);

		if (m_state == kClosed || m_vars[TK_CLOSE])
		{
			return Event(TK_CLOSE);
		}

		bool timed_out = false;

		if (timeout > 0)
		{
			timed_out = !m_input_condvar.wait_for
			(
				lock,
				std::chrono::milliseconds(timeout),
				[&](){return HasInputInternalUnlocked();}
			);
		}

		if ((timeout > 0 && !timed_out) || (timeout == 0 && HasInputInternalUnlocked()))
		{
			Event event = m_input_queue.front();
			ConsumeEvent(event);
			m_input_queue.pop_front();
			ConsumeIrrelevantEvents();
			return event;
		}
		else if (m_state == kClosed)
		{
			// State may change while waiting for a condvar
			return Event(TK_CLOSE);
		}
		else
		{
			// Instance is not closed and input either timed out or was not present from the start
			return Event(TK_INPUT_NONE);
		}
	}

	int Terminal::Read()
	{
		return ReadEvent(std::numeric_limits<int>::max()).code;
	}

	/**
	 * Reads whole string.
	 */
	int Terminal::ReadString(int x, int y, wchar_t* buffer, int max)
	{
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
			Print(x, y, buffer);
			if (put_cursor && cursor < max) Put(x+cursor, y, m_options.input_cursor_symbol);
		};

		auto restore_scene = [&]()
		{
			std::lock_guard<std::mutex> guard(m_lock);
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

			auto event = ReadEvent(m_options.input_cursor_blink_rate);

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
			//else if (m_vars[TK_WCHAR])
			else if (wchar_t ch = GetState(TK_WCHAR))
			{
				if (cursor < max)
				{
					//buffer[cursor++] = (wchar_t)m_vars[TK_WCHAR];
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

	const Encoding<char>& Terminal::GetEncoding() const
	{
		return *m_encoding;
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
	}

	void Terminal::PrepareFreshCharacters()
	{
		for (auto code: m_fresh_codes)
		{
			if (m_world.tiles.slots.find(code) != m_world.tiles.slots.end())
			{
				// Might be already prepared on previous iteration.
				continue;
			}

			bool provided = false;

			// Box Drawing (2500–257F) and Block Elements (2580–259F) are searched in different order
			bool is_dynamic = (code >= 0x2500 && code <= 0x257F) || (code >= 0x2580 && code <= 0x259F);

			for (auto i=m_world.tilesets.rbegin(); i != m_world.tilesets.rend(); i++)
			{
				if (is_dynamic)
				{
					// While searching for dynamic character provider, skip:
					// * dynamic tileset at 0xFFFD base code
					// * truetype basic tileset at 0x0000 code

					bool unsuitable =
						(i->first == kUnicodeReplacementCharacter) ||
						((i->first == 0x0000 && i->second->GetType() == Tileset::Type::TrueType));

					if (unsuitable) continue;
				}
				else if (i->second->Provides(code))
				{
					i->second->Prepare(code);
					provided = true;
					break;
				}
			}

			// If nothing was found, use dynamic tileset as a last resort
			if (!provided && m_world.tilesets[kUnicodeReplacementCharacter]->Provides(code))
			{
				m_world.tilesets[kUnicodeReplacementCharacter]->Prepare(code);
			}
		}

		m_fresh_codes.clear();
	}

	int Terminal::Redraw(bool async)
	{
		//*
		// Rendering callback will try to acquire the lock. Failing to  do so
		// will mean that Terminal is currently busy. Calling window implementation
		// SHOULD be prepared to reschedule painting to a later time.
		std::unique_lock<std::mutex> guard(m_lock, std::try_to_lock);
		if (!guard.owns_lock())
		{
			return -1; // TODO: enum TODO: timed try
		}
		/*/
		std::unique_lock<std::mutex> guard(m_lock);
		//*/

		// Provide tile slots for newly added codes
		if (!m_fresh_codes.empty())
		{
			PrepareFreshCharacters();
		}

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

		m_world.tiles.atlas.Refresh();
		Texture::Enable();

		int w2 = m_world.state.half_cellsize.width;
		int h2 = m_world.state.half_cellsize.height;
		bool layer_scissors_applied = false;

		uint64_t current_texture_id = 0;
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
						auto j = m_world.tiles.slots.find(leaf.code);

						if (j == m_world.tiles.slots.end())
						{
							// Replacement character MUST be provided by the ever-present dynamic tileset.
							j = m_world.tiles.slots.find(kUnicodeReplacementCharacter);
						}

						auto& slot = *(j->second);
						if (slot.texture_id != current_texture_id)
						{
							glEnd();
							slot.BindTexture();
							current_texture_id = slot.texture_id;
							glBegin(GL_QUADS);
						}

						slot.Draw(leaf, left, top, w2, h2);
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

	/**
	 * NOTE: if window initialization has succeeded, this callback will be called for sure
	 */
	void Terminal::HandleDestroy()
	{
		std::lock_guard<std::mutex> guard(m_lock);
		m_state = kClosed;

		// Dispose of graphics
		m_world.tiles.slots.clear();
		m_world.tilesets.clear();
		m_world.tiles.atlas.Dispose();

		// Unblock possibly blocked client thread
		m_input_condvar.notify_all();
	}

	int Terminal::OnWindowEvent(Event event)
	{
		bool alt = get_locked(m_vars[TK_ALT], m_input_lock);

		if (event.code == TK_DESTROY)
		{
			HandleDestroy();
			return 0;
		}
		else if (event.code == TK_REDRAW)
		{
			return Redraw(true);
		}
		else if (event.code == TK_INVALIDATE)
		{
			std::lock_guard<std::mutex> guard(m_lock);
			m_viewport_modified = true;
			return 0;
		}
		else if (event.code == TK_ACTIVATED)
		{
			std::lock_guard<std::mutex> guard(m_input_lock);

			// Cancel all pressed keys
			for (int i = 0; i <= TK_ALT; i++)
			{
				if (i == TK_CLOSE) continue;
				if (m_vars[i])
				{
					m_input_queue.push_back(Event(i|TK_KEY_RELEASED, {{i, 0}}));
				}
			}

			ConsumeIrrelevantEvents();
			return 0;
		}
		else if (event.code == TK_STATE_UPDATE)
		{
			ConsumeEvent(event);
			return 0;
		}
		else if (event.code == TK_RESIZED)
		{
			std::lock_guard<std::mutex> guard(m_lock);

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
			std::lock_guard<std::mutex> guard(m_lock);

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
			Point location
			(
				std::floor(pixel_x/(float)cellsize.width),
				std::floor(pixel_y/(float)cellsize.height)
			);

			if (location.x < 0)
			{
				location.x = 0;
				pixel_x = 0;
			}
			else if (location.x >= m_world.stage.size.width)
			{
				location.x = m_world.stage.size.width-1;
				pixel_x = m_world.stage.size.width*cellsize.width-1;
			}

			if (location.y < 0)
			{
				location.y = 0;
				pixel_y = 0;
			}
			else if (location.y >= m_world.stage.size.height)
			{
				location.y = m_world.stage.size.height-1;
				pixel_y = m_world.stage.size.height*cellsize.height-1;
			}

			if (!m_options.input_precise_mouse && m_vars[TK_MOUSE_X] == location.x && m_vars[TK_MOUSE_Y] == location.y)
			{
				// This event do not change mouse cell position, ignore.
				return 0;
			}
			else
			{
				// Make event update both pixel and cell positions.
				event[TK_MOUSE_X] = location.x;
				event[TK_MOUSE_Y] = location.y;
			}
		}
		else if (event.code == TK_A && alt)
		{
			// Alt+A: dump atlas textures to disk.
			std::lock_guard<std::mutex> guard(m_lock);
			m_world.tiles.atlas.Dump();
			return 0;
		}
		else if (event.code == TK_G && alt)
		{
			// Alt+G: toggle grid
			m_show_grid = !m_show_grid;
			Redraw(true);
			return 0;
		}
		else if (event.code == TK_RETURN && alt)
		{
			// Alt+ENTER: toggle fullscreen.
			m_viewport_modified = true;
			m_window->ToggleFullscreen();
			return 0;
		}
		else if ((event.code == TK_MINUS || event.code == TK_EQUALS || event.code == TK_KP_MINUS || event.code == TK_KP_PLUS) && alt)
		{
			if (get_locked(m_options.window_resizeable, m_lock))
			{
				// No scaling in resizeable mode, at least not yet.
				return 0;
			}

			// Alt+(plus/minus): adjust user window scaling.
			if ((event.code == TK_MINUS || event.code == TK_KP_MINUS) && m_scale_step > 0)
			{
				m_scale_step -= 1;
			}
			else if ((event.code == TK_EQUALS || event.code == TK_KP_PLUS) && m_scale_step < kScaleSteps.size()-1)
			{
				m_scale_step += 1;
			}

			m_window->SetSizeHints(m_world.state.cellsize * kScaleSteps[m_scale_step], m_options.window_minimum_size);
			m_window->SetClientSize(m_world.state.cellsize * m_world.stage.size * kScaleSteps[m_scale_step]);

			return 0;
		}
		else if ((event.code & 0xFF) == TK_ALT)
		{
			std::lock_guard<std::mutex> guard(m_input_lock);
			ConsumeEvent(event);
			return 0;
		}

		std::lock_guard<std::mutex> guard(m_input_lock);
		m_input_queue.push_back(std::move(event));

		ConsumeIrrelevantEvents();

		if (!m_input_queue.empty())
		{
			m_input_condvar.notify_all();
		}

		return 0;
	}

	void Terminal::ConsumeEvent(Event& event)
	{
		if (event.code == TK_RESIZED)
		{
			std::lock_guard<std::mutex> guard(m_lock);

			if (m_options.window_resizeable)
			{
				// Stage size changed, must reallocate and reconstruct scene
				m_options.window_size = Size(event[TK_WIDTH], event[TK_HEIGHT]);
				m_world.stage.Resize(m_options.window_size);

				// Client size changed, must redraw
				m_viewport_modified = true;
			}
			else
			{
				return;
			}
		}
		else if (event.code == TK_CLOSE)
		{
			std::lock_guard<std::mutex> guard(m_lock);

			if (m_options.input_sticky_close)
			{
				m_vars[TK_CLOSE] = 1;
			}
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

	void Terminal::ConsumeIrrelevantEvents()
	{
		while (!m_input_queue.empty())
		{
			Event& event = m_input_queue.front();

			bool must_be_consumed =
				(event.domain == Event::Domain::Internal) ||
				(event.domain == Event::Domain::Keyboard && !m_options.input_keyboard) ||
				(event.domain == Event::Domain::Mouse && !m_options.input_mouse);

			if (must_be_consumed)
			{
				ConsumeEvent(event);
				m_input_queue.pop_front();
			}
			else
			{
				break;
			}
		}
	}
}
