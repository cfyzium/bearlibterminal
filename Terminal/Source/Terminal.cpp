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

namespace BearLibTerminal
{
	Terminal::Terminal():
		m_state{kHidden},
		m_vars{},
		m_asynchronous{true},
		m_current_texture(0),
		m_inside_drawing_block(false)
	{
		// Try to create window
		m_window = Window::Create();
		m_window->SetOnDestroy(std::bind(&Terminal::OnWindowClose, this));
		m_window->SetOnInput(std::bind(&Terminal::OnWindowInput, this, std::placeholders::_1));
		m_window->SetOnRedraw(std::bind(&Terminal::OnWindowRedraw, this));
		m_window->SetOnActivate(std::bind(&Terminal::OnWindowActivate, this));

		// Default parameters
		SetOptionsInternal(L"window: size=40x25; font: default; terminal.encoding=utf8");
	}

	Terminal::~Terminal()
	{
		if (!m_asynchronous && m_window)
		{
			// Must rebind OpenGL context to window thread for safe deinitialization
			SwitchRenderingThread(true);
		}

		m_window.reset();
	}

	void Terminal::InvokeOnRenderingThread(std::function<void()> func)
	{
		if (m_asynchronous)
		{
			// Window thread owns rendering context
			m_window->Invoke(func);
		}
		else
		{
			// Terminal (current thread) owns rendering context
			func();
		}
	}

	void Terminal::SwitchRenderingThread(bool window)
	{
		InvokeOnRenderingThread([&]{m_window->ReleaseRC();});
		m_asynchronous = window;
		InvokeOnRenderingThread([&]{m_window->AcquireRC();});
	}

	void Terminal::LeaveDrawingBlock()
	{
		if (!m_asynchronous && m_inside_drawing_block)
		{
			glEnd();
			m_inside_drawing_block = false;
		}
	}

	void Terminal::EnterDrawingBlock()
	{
		if (!m_asynchronous && !m_inside_drawing_block)
		{
			m_world.tiles.atlas.Refresh();
			glBegin(GL_QUADS);
			m_inside_drawing_block = true;
		}
	}

	int Terminal::SetOptions(const std::wstring& value)
	{
		LOG(Info, "Trying to set \"" << value << "\"");
		try
		{
			SetOptionsInternal(value);
			return 0;
		}
		catch (std::exception& e)
		{
			LOG(Error, "Failed to set some options: " << e.what());
			return -1;
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
		if (!m_asynchronous) LeaveDrawingBlock();

		auto groups = ParseOptions(value);

		Options updated = m_options;
		std::map<uint16_t, std::unique_ptr<Tileset>> new_tilesets;

		// Validate options
		for (auto& group: groups)
		{
			//*
			// DEBUG: dump parsed options to file
			LOG(Info, L"Group \"" << group.name << "\":");
			for (auto attribute: group.attributes)
			{
				LOG(Info, L"  \"" << attribute.first << "\" = \"" << attribute.second << "\"");
			}
			//*/

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

		if (updated.output_asynchronous != m_options.output_asynchronous)
		{
			LOG(Debug, "Switching to " << (updated.output_asynchronous? "asynchronous": "synchronous") << " rendering mode");
			SwitchRenderingThread(updated.output_asynchronous);
		}

		if (updated.output_vsync != m_options.output_vsync)
		{
			InvokeOnRenderingThread([&]{m_window->SetVSync(updated.output_vsync);});
		}

		// All options and parameters must be validated, may try to apply them
		if (!new_tilesets.empty())
		{
			InvokeOnRenderingThread([&](){ApplyTilesets(new_tilesets);});
		}

		// Primary sanity check: if there is no base font, lots of things are gonna fail
		if (!m_world.tilesets.count(0))
		{
			throw std::runtime_error("No base font has been configured");
		}

		// Such implementation is awful. Should use some global (external library?) instance.
		if (updated.log_filename != m_options.log_filename) g_log.SetFile(updated.log_filename);
		if (updated.log_level != m_options.log_level) g_log.SetLevel(updated.log_level);
		if (updated.log_mode != m_options.log_mode) g_log.SetMode(updated.log_mode);

		if (updated.terminal_encoding != m_options.terminal_encoding)
		{
			m_encoding = GetUnibyteEncoding(updated.terminal_encoding);
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
			InvokeOnRenderingThread([=](){UpdateDynamicTileset(m_world.state.cellsize);});

			viewport_size_changed = true;
			LOG(Debug, L"SetOptions: new cell size is " << m_world.state.cellsize);
		}

		if (updated.window_size != m_options.window_size)
		{
			// Update window size: resize the stage
			m_world.stage.Resize(updated.window_size);
			viewport_size_changed = true;
			LOG(Debug, L"SetOptions: new window size is " << updated.window_size);
		}

		if (viewport_size_changed)
		{
			// Resize window object
			Size viewport_size = m_world.stage.size * m_world.state.cellsize;
			m_window->SetClientSize(viewport_size);
			InvokeOnRenderingThread([=](){ConfigureViewport();});
		}

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
	}

	void Terminal::ValidateInputOptions(OptionGroup& group, Options& options)
	{
		// Possible options: nonblocking, events, precise_mouse, sticky_close, cursor_symbol, cursor_blink_rate

		if (group.attributes.count(L"nonblocking") && !try_parse(group.attributes[L"nonblocking"], options.input_nonblocking))
		{
			throw std::runtime_error("input.nonblocking cannot be parsed");
		}

		if (group.attributes.count(L"precise_mouse") && !try_parse(group.attributes[L"precise_mouse"], options.input_precise_mouse))
		{
			throw std::runtime_error("input.precise_mouse cannot be parsed");
		}

		if (group.attributes.count(L"sticky_close") && !try_parse(group.attributes[L"sticky_close"], options.input_sticky_close))
		{
			throw std::runtime_error("input.sticky_close cannot be parsed");
		}

		if (group.attributes.count(L"events"))
		{
			const std::wstring& s = group.attributes[L"events"];
			std::map<std::wstring, uint32_t> flags
			{
				{L"all", (uint32_t)InputEvents::All},
				{L"keypress", (uint32_t)InputEvents::KeyPress},
				{L"keyrelease", (uint32_t)InputEvents::KeyRelease},
				{L"mousemove", (uint32_t)InputEvents::MouseMove},
				{L"mousescroll", (uint32_t)InputEvents::MouseScroll},
				{L"none", (uint32_t)InputEvents::None}
			};

			uint32_t result = 0;
			for (auto i: flags)
			{
				size_t n = s.find(i.first);
				if (n != std::wstring::npos)
				{
					uint32_t value = i.second;
					if (n > 0 && s[n-1] == L'-') result &= ~value; else result |= value;
				}
			}

			options.input_events = result;
		}

		if (group.attributes.count(L"cursor_symbol") && !try_parse(group.attributes[L"cursor_symbol"], options.input_cursor_symbol))
		{
			throw std::runtime_error("input.cursor_symbol cannot be parsed");
		}

		if (group.attributes.count(L"cursor_blink_rate") && !try_parse(group.attributes[L"cursor_blink_rate"], options.input_cursor_blink_rate))
		{
			throw std::runtime_error("input.cursor_blink_rate cannot be parsed");
		}

		if (options.input_cursor_blink_rate <= 0) options.input_cursor_blink_rate = 1;
	}

	void Terminal::ValidateOutputOptions(OptionGroup& group, Options& options)
	{
		// Possible options: postformatting, synchronous, vsync

		if (group.attributes.count(L"postformatting") && !try_parse(group.attributes[L"postformatting"], options.output_postformatting))
		{
			throw std::runtime_error("output.postformatting cannot be parsed");
		}

		if (group.attributes.count(L"asynchronous") && !try_parse(group.attributes[L"asynchronous"], options.output_asynchronous))
		{
			throw std::runtime_error("output.asynchronous cannot be parsed");
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

	void Terminal::Refresh()
	{
		if (m_asynchronous)
		{
			// FIXME: m_state here is not protected by lock because
			// Window::Show will try to repaint syncronously which causes deadlock.
			// This additional repaint is unnecessary.

			// If window is not visible, show it
			if (m_state == kHidden)
			{
				m_window->Show();
				m_state = kVisible;
			}

			// Ignore irrelevant redraw calls (in case something has failed and
			// state is already kClosed).
			if (m_state != kVisible) return;

			// Synchronously copy backbuffer to frontbuffer
			{
				std::lock_guard<std::mutex> guard(m_lock);
				m_world.stage.frontbuffer = m_world.stage.backbuffer;
			}

			// NOTE: this call will wait OnWindowRedraw completion
			m_window->Redraw();
		}
		else
		{
			LeaveDrawingBlock();
			// NOTE: debug overlays go here
			//m_window->Invoke([]{});
			m_window->SwapBuffers();
		}
	}

	void Terminal::Clear()
	{
		if (m_asynchronous)
		{
			std::lock_guard<std::mutex> guard(m_lock);
			m_world.stage.Resize(m_world.stage.size);
		}
		else
		{
			LeaveDrawingBlock();
			glClear(GL_COLOR_BUFFER_BIT);
		}
	}

	void Terminal::Clear(int x, int y, int w, int h)
	{
		if (m_asynchronous)
		{
			std::lock_guard<std::mutex> guard(m_lock);

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
					int i = stage_size.width*j+i;
					layer.cells[i].leafs.clear();
					if (m_world.state.layer == 0)
					{
						m_world.stage.backbuffer.background[i] = m_world.state.bkcolor;
					}
				}
			}
		}
		else
		{
			Size stage_size = m_world.stage.size;
			Size cell_size = m_world.state.cellsize;
			if (x < 0) x = 0;
			if (y < 0) y = 0;
			if (x+w >= stage_size.width) w = stage_size.width-x;
			if (y+h >= stage_size.height) h = stage_size.height-y;

			LeaveDrawingBlock();
			glEnable(GL_SCISSOR_TEST);
			glScissor
			(
				x * cell_size.width,
				(stage_size.height - (y+1)) * cell_size.height,
				w * cell_size.width,
				h * cell_size.height
			);
			glClear(GL_COLOR_BUFFER_BIT);
			glDisable(GL_SCISSOR_TEST);
		}
	}

	void Terminal::SetLayer(int layer_index)
	{
		// Layer index is limited to [0..255]
		if (layer_index < 0) layer_index = 0;
		if (layer_index > 255) layer_index = 255;
		m_world.state.layer = layer_index;

		std::lock_guard<std::mutex> guard(m_lock);
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

	void Terminal::PutUnlocked(int x, int y, int dx, int dy, wchar_t code, Color* colors)
	{
		if (m_asynchronous)
		{
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
				if (m_world.state.composition == TK_COMPOSITION_OFF)
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
				if (m_world.state.layer == 0)
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
		else
		{
			// TODO: easier leaf construct, more direct drawing procedure
			Leaf leaf;
			leaf.dx = dx;
			leaf.dy = dy;

			if (colors)
			{
				for (int i=0; i<4; i++) leaf.color[i] = colors[i];
				leaf.flags |= Leaf::CornerColored;
			}
			else
			{
				leaf.color[0] = m_world.state.color;
			}

			auto j = m_world.tiles.slots.find(code);
			if (j == m_world.tiles.slots.end())
			{
				LOG(Trace, "Trying to prepare character " << (int)code << " in synchronous mode");
				LeaveDrawingBlock();
				m_fresh_codes.emplace_back(code);
				PrepareFreshCharacters();
				j = m_world.tiles.slots.find(code);
			}

			auto& slot = *(j->second);
			if (slot.texture_id != m_current_texture)
			{
				LOG(Trace, "Wrong texture is currently bound, rebinding while in synchornous mode");
				LeaveDrawingBlock();
				slot.BindTexture();
				m_current_texture = slot.texture_id;
			}

			EnterDrawingBlock();
			slot.Draw
			(
				leaf,
				m_world.state.cellsize.width * x,
				m_world.state.cellsize.height * y,
				m_world.state.half_cellsize.width,
				m_world.state.half_cellsize.height
			);
		}
	}

	void Terminal::Put(int x, int y, wchar_t code)
	{
		std::lock_guard<std::mutex> guard(m_lock);
		if (x < 0 || y < 0 || x >= m_world.stage.size.width || y >= m_world.stage.size.height) return;
		PutUnlocked(x, y, 0, 0, code, nullptr);
	}

	void Terminal::PutExtended(int x, int y, int dx, int dy, wchar_t code, Color* corners)
	{
		std::lock_guard<std::mutex> guard(m_lock);
		if (x < 0 || y < 0 || x >= m_world.stage.size.width || y >= m_world.stage.size.height) return;
		PutUnlocked(x, y, dx, dy, code, corners);
	}

	void Terminal::CustomRendering(int mode)
	{
		if (m_asynchronous) return;

		if (mode == 1)
		{
			// Enter custom rendering, stop current rendering block
			LeaveDrawingBlock();
		}
		else
		{
			// Leave custom rendering, resume rendering block.
			// Rebuild matrices and reapply blending modes.
			ConfigureViewport();

			// Texture must be reset so it can be properly reapplied later.
			glEnable(GL_TEXTURE_2D);
			m_current_texture = 0;
		}
	}

	int Terminal::Print(int x0, int y0, const std::wstring& s)
	{
		std::lock_guard<std::mutex> guard(m_lock);

		int x = x0, y = y0;
		int printed = 0;
		uint16_t base = 0;
		Encoding<char>* codepage = nullptr;

		Color original_color = m_world.state.color;
		Color original_bkcolor = m_world.state.bkcolor;

		Size size = m_world.stage.size;

		const auto put_and_increment = [&](int code)
		{
			if (x >=0 && y >= 0 && x < size.width && y < size.height)
			{
				// Convert from unicode to tileset codepage
				if (codepage) code = codepage->Convert((wchar_t)code);

				// Offset tile index
				code += base;

				PutUnlocked(x, y, 0, 0, code, nullptr);
				printed += 1;
			}

			x += 1;
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
				else if (name[0] == 'u' || name[0] == 'U')
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
				y += 1;
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

	int Terminal::HasInput()
	{
		std::lock_guard<std::mutex> guard(m_lock);
		if (m_state == kClosed) return 1;
		if (m_vars[TK_CLOSE] && m_options.input_sticky_close) return 1; // sticky VK_CLOSE, once set, can't be undone
		return !m_input_queue.empty();
	}

	int Terminal::GetState(int code)
	{
		std::lock_guard<std::mutex> guard(m_lock);
		return (code >= 0 && code < (int)m_vars.size())? m_vars[code]: 0;
	}

	Keystroke Terminal::ReadKeystroke(int timeout)
	{
		std::unique_lock<std::mutex> lock(m_lock);

		// Sticky close cannot be undone once set
		if (m_vars[TK_CLOSE] && m_options.input_sticky_close)
		{
			return Keystroke(TK_CLOSE);
		}

		// Keep terminal from blocking accidentally if input.events is None
		if (m_options.input_events == InputEvents::None)
		{
			return Keystroke(0);
		}

		if (timeout > 0)
		{
			auto predicate = [&]() -> bool {return !m_input_queue.empty() || (m_state == kClosed);};
			m_input_condvar.wait_for(lock, std::chrono::milliseconds(timeout), predicate);
		}

		if (!m_input_queue.empty())
		{
			Keystroke stroke = m_input_queue.front();
			m_input_queue.pop_front();
			ConsumeStroke(stroke);
			return stroke;
		}
		else if (m_state == kClosed)
		{
			return Keystroke(TK_CLOSE);
		}
		else
		{
			return Keystroke(0);
		}
	}

	/**
	 * Read any non-filtered input event
	 */
	int Terminal::Read()
	{
		bool nonblocking = get_locked(m_options.input_nonblocking, m_lock);
		if (!m_asynchronous) nonblocking = true;
		Keystroke stroke = ReadKeystroke(nonblocking? 0: std::numeric_limits<int>::max());
		int result = stroke.scancode;
		if (stroke.released) result |= TK_FLAG_RELEASED;
		return result;
	}

	int Terminal::ReadCharInternal(int timeout)
	{
		do
		{
			Keystroke stroke = ReadKeystroke(timeout);
			if (stroke.scancode == TK_CLOSE)
			{
				// Break on VK_CLOSE but push it back so subsequent Read could return it
				std::unique_lock<std::mutex> lock(m_lock);
				m_input_queue.push_front(Keystroke(TK_CLOSE));
				ConsumeIrrelevantInput();
				return -1;
			}
			if (stroke.scancode == TK_ESCAPE)
			{
				// Just break
				return TK_INPUT_CANCELLED;
			}
			else if (stroke.scancode == 0)
			{
				// No input available
				return TK_INPUT_CALL_AGAIN;
			}
			else if (stroke.character > 0 && !stroke.released)
			{
				// Textual key-down event with
				return stroke.character;
			}
		}
		while (true);
	}

	/**
	 * Read first character event
	 */
	int Terminal::ReadChar()
	{
		bool nonblocking = get_locked(m_options.input_nonblocking, m_lock);
		if (!m_asynchronous) nonblocking = true;
		return ReadCharInternal(nonblocking? 0: std::numeric_limits<int>::max());
	}

	int Terminal::ReadStringInternalBlocking(int x, int y, wchar_t* buffer, int max)
	{
		std::vector<Cell> original;
		int composition_mode = m_world.state.composition;
		m_world.state.composition = TK_COMPOSITION_ON;

		// Syncronously retrieve/adjust some values
		{
			std::lock_guard<std::mutex> guard(m_lock);

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

			auto key = ReadKeystroke(m_options.input_cursor_blink_rate);
			if (key.scancode == 0)
			{
				// Timed out
				show_cursor = !show_cursor;
			}
			else if (!key.released)
			{
				if (key.character > 0)
				{
					if (cursor < max)
					{
						buffer[cursor++] = key.character;
						show_cursor = true;
					}
				}
				else if (key.scancode == TK_RETURN)
				{
					rc = wcslen(buffer);
					break;
				}
				else if (key.scancode == TK_ESCAPE || key.scancode == TK_CLOSE)
				{
					rc = TK_INPUT_CANCELLED;
					break;
				}
				else if (key.scancode == TK_BACK)
				{
					if (cursor > 0)
					{
						buffer[--cursor] = 0;
						show_cursor = true;
					}
				}
			}
		}

		// Restore
		restore_scene();
		m_world.state.composition = composition_mode;

		return rc;
	}

	int Terminal::ReadStringInternalNonblocking(wchar_t* buffer, int max)
	{
		if (buffer == nullptr || max <= 0)
		{
			LOG(Error, "Invalid buffer parameters were passed to string reading function");
			return 0;
		}

		// Garbage string protection
		for (int i=0, f=0; i<max+1; i++) if (f) buffer[i] = 0; else f = !buffer[i];
		buffer[max] = 0;
		int cursor = std::wcslen(buffer);
		int rc = 0;

		while (true)
		{
			auto key = ReadKeystroke(0);
			if (key.scancode == 0)
			{
				rc = TK_INPUT_CALL_AGAIN;
				break;
			}
			else if (!key.released)
			{
				if (key.character > 0)
				{
					if (cursor < max)
					{
						buffer[cursor++] = key.character;
					}
				}
				else if (key.scancode == TK_RETURN)
				{
					rc = wcslen(buffer);
					break;
				}
				else if (key.scancode == TK_ESCAPE || key.scancode == TK_CLOSE)
				{
					rc = TK_INPUT_CANCELLED;
					break;
				}
				else if (key.scancode == TK_BACK)
				{
					if (cursor > 0)
					{
						buffer[--cursor] = 0;
					}
				}
			}
		}

		return rc;
	}

	/**
	 * Reads whole string. Operates vastly differently in blocking and non-blocking modes
	 */
	int Terminal::ReadString(int x, int y, wchar_t* buffer, int max)
	{
		return (m_options.input_nonblocking || !m_asynchronous)?
			ReadStringInternalNonblocking(buffer, max):
			ReadStringInternalBlocking(x, y, buffer, max);
	}

	const Encoding<char>& Terminal::GetEncoding() const
	{
		return *m_encoding;
	}

	void Terminal::ConfigureViewport()
	{
		Size viewport_size = m_world.stage.size * m_world.state.cellsize;

		glClearColor(0, 0, 0, 1);
		glViewport(0, 0, viewport_size.width, viewport_size.height);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, viewport_size.width, viewport_size.height, 0, -1, +1);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
					// While searching for dynamic character provider, skip (at leaft for now):
					// * dynamic tileset at 0xFFFD base code
					// * basic tileset at 0x0000 code (only if it is of TrueType type)

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
				provided = true;
			}

			if (!provided)
			{
				// Use Unicode replacement character code (MUST be already provided by dynamic tileset)
				m_world.tiles.slots[code] = m_world.tiles.slots[kUnicodeReplacementCharacter];
			}
		}

		m_fresh_codes.clear();
	}

	/**
	 * NOTE: if window initialization has succeeded, this callback will be called for sure
	 */
	void Terminal::OnWindowClose()
	{
		LOG(Debug, "OnWindowClose callback is called");

		std::lock_guard<std::mutex> guard(m_lock);
		m_state = kClosed;

		// Dispose of graphics
		m_world.tiles.slots.clear();
		m_world.tilesets.clear();
		m_world.tiles.atlas.Dispose();

		// Unblock possibly blocked client thread
		m_input_condvar.notify_all();
	}

	bool Terminal::OnWindowRedraw()
	{
		// Scene-based rendering function is used in asyncronous mode only
		if (!m_asynchronous) return false;

		std::unique_lock<std::mutex> guard(m_lock);

		// Provide tile slots for newly added codes
		if (!m_fresh_codes.empty())
		{
			PrepareFreshCharacters();
		}

		glClear(GL_COLOR_BUFFER_BIT);

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

		uint64_t current_texture_id = 0;
		glBegin(GL_QUADS);
		glColor4f(1, 1, 1, 1);
		for (auto& layer: m_world.stage.frontbuffer.layers)
		{
			int i = 0, left = 0, top = 0;

			for (int y=0; y<m_world.stage.size.height; y++)
			{
				for (int x=0; x<m_world.stage.size.width; x++)
				{
					for (auto& leaf: layer.cells[i].leafs)
					{
						auto j = m_world.tiles.slots.find(leaf.code);
						if (j != m_world.tiles.slots.end())
						{
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
						else
						{
							LOG(Debug, "Didn't find slot for code " << leaf.code);
						}
					}

					i += 1;
					left += m_world.state.cellsize.width;
				}

				left = 0;
				top += m_world.state.cellsize.height;
			}
		}
		glEnd();

		/*
		// Ghost
		glBegin(GL_QUADS);
		glColor4f(1, 1, 1, 0.25f);
		glTexCoord2f(0, 0);
		glVertex2i(16, 16);
		glTexCoord2f(0, 1);
		glVertex2i(16, 16+256);
		glTexCoord2f(1, 1);
		glVertex2i(16+256, 16+256);
		glTexCoord2f(1, 0);
		glVertex2i(16+256, 16);
		glEnd();
		//*/

		return true;
	}

	void Terminal::OnWindowInput(Keystroke keystroke)
	{
		std::lock_guard<std::mutex> guard(m_lock);
		m_input_queue.push_back(keystroke);
		ConsumeIrrelevantInput();
		if (!m_input_queue.empty())
		{
			m_input_condvar.notify_all();
		}
	}

	void Terminal::ConsumeIrrelevantInput()
	{
		auto filter = m_options.input_events;

		while (!m_input_queue.empty())
		{
			Keystroke& stroke = m_input_queue.front();
			bool must_be_consumed = false;

			if (filter == InputEvents::None)
			{
				must_be_consumed = true;
			}
			else if (stroke.scancode == TK_MOUSE_MOVE)
			{
				if (!(filter & InputEvents::MouseMove))
				{
					must_be_consumed = true;
				}
				else if (!m_options.input_precise_mouse)
				{
					// Check if mouse cursor changed cell location
					float cell_width = m_vars[TK_CELL_WIDTH];
					float cell_height = m_vars[TK_CELL_HEIGHT];
					int mx = (int)std::floor(stroke.x/cell_width);
					int my = (int)std::floor(stroke.y/cell_height);
					if (mx == m_vars[TK_MOUSE_X] && my == m_vars[TK_MOUSE_Y])
					{
						must_be_consumed = true;
					}
				}
			}
			else if (stroke.scancode == TK_MOUSE_SCROLL)
			{
				if (!(filter & InputEvents::MouseScroll))
				{
					must_be_consumed = true;
				}
			}
			else
			{
				if (!stroke.released)
				{
					if (!(filter & InputEvents::KeyPress))
					{
						must_be_consumed = true;
					}
				}
				else
				{
					if (!(filter & InputEvents::KeyRelease))
					{
						must_be_consumed = true;
					}
				}
			}

			if (!must_be_consumed) break;

			ConsumeStroke(stroke);
			m_input_queue.pop_front();
			continue;
		}
	}

	void Terminal::ConsumeStroke(const Keystroke& stroke)
	{
		if (stroke.scancode == TK_MOUSE_MOVE)
		{
			// Mouse movement event, update mouse position
			float cell_width = m_vars[TK_CELL_WIDTH];
			float cell_height = m_vars[TK_CELL_HEIGHT];
			m_vars[TK_MOUSE_X] = (int)std::floor(stroke.x/cell_width);
			m_vars[TK_MOUSE_Y] = (int)std::floor(stroke.y/cell_height);
			m_vars[TK_MOUSE_PIXEL_X] = stroke.x;
			m_vars[TK_MOUSE_PIXEL_Y] = stroke.y;
		}
		else if (stroke.scancode == TK_MOUSE_SCROLL)
		{
			// Mouse scroll event, update wheel position
			m_vars[TK_MOUSE_WHEEL] = stroke.z;
		}
		else if (stroke.scancode == TK_CLOSE)
		{
			if (m_options.input_sticky_close || !(m_options.input_events & InputEvents::KeyPress))
			{
				m_vars[TK_CLOSE] = 1;
			}
		}
		else
		{
			m_vars[stroke.scancode] = (int)!stroke.released;
		}
	}

	void Terminal::OnWindowActivate()
	{
		// Cancel all pressed keys
		for ( int i = 0; i <= TK_F12; i++ ) // NOTE: TK_F12 is the last physical key index
		{
			if (i == TK_CLOSE) continue;
			if (m_vars[i]) OnWindowInput(Keystroke(i, true));
		}
	}
}
