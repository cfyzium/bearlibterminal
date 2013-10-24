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
#include "BearLibTerminal.h"
#include <cmath>

namespace BearLibTerminal
{
	Terminal::Terminal():
		m_state{kHidden},
		m_vars{},
		m_synchronous{false}
	{
		// Try to create window
		m_window = Window::Create();
		m_window->SetOnDestroy(std::bind(&Terminal::OnWindowClose, this));
		m_window->SetOnInput(std::bind(&Terminal::OnWindowInput, this, std::placeholders::_1));
		m_window->SetOnRedraw(std::bind(&Terminal::OnWindowRedraw, this));
		m_window->SetOnActivate(std::bind(&Terminal::OnWindowActivate, this));

		// Layer 0 must always be present
		m_world.stage.backbuffer.layers.push_back(Layer());

		// Default parameters
		SetOptionsInternal(L"window: size=40x25; font: default");

		// FIXME: Setup default encoding
	}

	Terminal::~Terminal()
	{
		m_window.reset();
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

	void Terminal::SetOptionsInternal(const std::wstring& value)
	{
		std::lock_guard<std::mutex> guard(m_lock);

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
				// ...
			}
			else
			{
				uint16_t base_code = 0; // Basic font base_code is 0
				if (group.name == L"font" || try_parse<uint16_t>(group.name, base_code))
				{
					new_tilesets[base_code] = Tileset::Create(m_world.tiles, group);
					LOG(Debug, "Successfully loaded a tileset for base code " << base_code);
				}
			}
		}

		// All options and parameters must be validated, may try to apply them
		if (!new_tilesets.empty())
		{
			m_window->Invoke([&](){ApplyTilesets(new_tilesets);}); // XXX: from rendering thread
		}

		// Primary sanity check: if there is no base font, lots of things are gonna fail
		if (!m_world.tilesets.count(0))
		{
			throw std::runtime_error("No base font has been configured");
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

		if (updated.window_cellsize != m_options.window_cellsize || new_tilesets.count(0))
		{
			// Refresh stage.state.cellsize
			m_world.state.cellsize = updated.window_cellsize;

			// If specified cellsize is nil, fall back on base font bounding box
			if (!m_world.state.cellsize.Area())
			{
				//  NOTE: by now, tileset container MUST have 0th tileset
				m_world.state.cellsize = m_world.tilesets[0]->GetBoundingBoxSize();
			}

			m_vars[VK_CELL_WIDTH] = m_world.state.cellsize.width; // TODO: move vars to world.state
			m_vars[VK_CELL_HEIGHT] = m_world.state.cellsize.height;

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
			m_window->Invoke([=](){ConfigureViewport();}); // XXX: from rendering thread
		}

		m_options = updated;
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
				throw std::runtime_error(L"window.icon cannot be loaded");
			}

			options.window_icon = group.attributes[L"icon"];
		}
	}

	void Terminal::Refresh()
	{
		// XXX: m_state in not under lock

		// If window is not visible, show it
		if (m_state == kHidden)
		{
			m_window->Show();
			m_state = kVisible;
		}

		// Ignore irrelevant redraw calls (in case something has failed and state is kClosed)
		if (m_state != kVisible) return;

		// Atomically replace frontbuffer
		{
			std::lock_guard<std::mutex> guard(m_lock);
			m_world.stage.frontbuffer = m_world.stage.backbuffer;
		}

		// NOTE: this call will syncronously wait OnWindowRedraw completion
		m_window->Redraw();
	}

	void Terminal::Clear()
	{
		std::lock_guard<std::mutex> guard(m_lock);
		m_world.stage.Resize(m_world.stage.size);
	}

	void Terminal::Clear(int x, int y, int w, int h)
	{
		// FIXME: NYI
	}

	void Terminal::SetLayer(int layer_index)
	{
		m_world.state.layer = layer_index;

		std::lock_guard<std::mutex> guard(m_lock);
		while (m_world.stage.backbuffer.layers.size() <= m_world.state.layer)
		{
			m_world.stage.backbuffer.layers.push_back(Layer());
			m_world.stage.backbuffer.layers.back().cells.resize(m_world.stage.size.Area()); // TODO: Layer constructor
		}
	}

	void Terminal::SetForeColor(Color color)
	{
		m_world.state.color = color;
	}

	void Terminal::SetBackColor(Color color)
	{
		m_world.state.bkcolor = color;
	}

	void Terminal::SetComposition(int mode)
	{
		m_world.state.composition = mode;
	}

	/**
	 * NOTE: this function does not check bounds
	 */
	void Terminal::PutUnlocked(int x, int y, wchar_t code)
	{
		uint16_t u16code = (uint16_t)code; // TODO: use either wchar_t or uintxx_t
		if (m_world.tiles.slots.find(u16code) == m_world.tiles.slots.end())
		{
			m_fresh_codes.push_back(u16code);
		}

		int index = y*m_world.stage.size.width+x;
		Cell& cell = m_world.stage.backbuffer.layers[m_world.state.layer].cells[index];

		// Character FIXME: composition mode
		cell.leafs.push_back(Leaf());
		cell.leafs.back().code = u16code;

		// Colors
		cell.leafs.back().color[0] = m_world.state.color;
		if (m_world.state.bkcolor.a > 0)
		{
			m_world.stage.backbuffer.background[index] = m_world.state.bkcolor;
		}
	}

	void Terminal::Put(int x, int y, wchar_t code)
	{
		std::lock_guard<std::mutex> guard(m_lock);
		if (x < 0 || y < 0 || x >= m_world.stage.size.width || y >= m_world.stage.size.height) return;
		PutUnlocked(x, y, code);
	}

	void Terminal::PutExtended(int x, int y, int dx, int dy, wchar_t code, Color* corners)
	{
		// FIXME: NYI
	}

	int Terminal::Print(int x, int y, const std::wstring& str)
	{
		std::lock_guard<std::mutex> guard(m_lock);

		LOG(Trace, "Printing \"" << str << "\"");
		for (auto c: str)
		{
			if (x >= m_world.stage.size.width) break;
			PutUnlocked(x, y, c);
			x += 1;
		}
	}

	int Terminal::HasInput()
	{
		std::lock_guard<std::mutex> guard(m_lock);
		if (m_state == kClosed) return 1;
		if (m_vars[VK_CLOSE] && m_options.input_sticky_close) return 1; // sticky VK_CLOSE, once set, can't be undone
		return !m_input_queue.empty();
	}

	int Terminal::GetState(int code)
	{
		std::lock_guard<std::mutex> guard(m_lock);
		return (code >= 0 && code < (int)m_vars.size())? m_vars[code]: 0;
	}

	Keystroke Terminal::ReadKeystroke()
	{
		std::unique_lock<std::mutex> lock(m_lock);

		// Sticky close cannot be undone once set
		if (m_vars[VK_CLOSE] && m_options.input_sticky_close)
		{
			return Keystroke(VK_CLOSE);
		}

		// Keep terminal from blocking accidentally if input.events is None
		if (m_options.input_events == InputEvents::None)
		{
			return Keystroke(0);
		}

		if (!m_options.input_nonblocking)
		{
			auto predicate = [&]() -> bool {return !m_input_queue.empty() || (m_state == kClosed);};
			m_input_condvar.wait(lock, predicate);
		}

		if (!m_input_queue.empty())
		{
			Keystroke stroke = m_input_queue.front();
			m_input_queue.pop_front();
			return stroke;
		}
		else if (m_state == kClosed)
		{
			return Keystroke(VK_CLOSE);
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
		Keystroke stroke = ReadKeystroke();
		int result = stroke.scancode;
		if (stroke.released) result |= VK_FLAG_RELEASED;
		return result;
	}

	/**
	 * Read first character event
	 */
	int Terminal::ReadChar()
	{
		// FIXME: NYI
	}

	/**
	 * Reads whole string. Operates vastly differently in blocking and non-blocking modes
	 */
	int Terminal::ReadString(int x, int y, wchar_t* buffer, int max)
	{
		// FIXME: NYI
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
			bool provided = false;
			for (auto i=m_world.tilesets.rbegin(); i != m_world.tilesets.rend(); i++)
			{
				if (i->second->Provides(code))
				{
					i->second->Prepare(code);
					provided = true;
					break;
				}
			}

			if (!provided)
			{
				// Use no-character code (MUST be already provided by dynamic tileset)
				// FIXME: NYI
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

		//*
		// Dispose of graphics
		m_world.tiles.slots.clear();
		m_world.tilesets.clear();
		m_world.tiles.atlas.Dispose();
		//*/

		// Unblock possibly blocked client thread
		m_input_condvar.notify_all();
	}

	void Terminal::OnWindowRedraw()
	{
		std::unique_lock<std::mutex> guard(m_lock);

		// Provide tile slots for newly added codes
		if (!m_fresh_codes.empty())
		{
			PrepareFreshCharacters();
		}

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
							slot.Draw(leaf, Point(left, top));
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
			else if (stroke.scancode == VK_MOUSE_MOVE)
			{
				if (!(filter & InputEvents::MouseMove))
				{
					must_be_consumed = true;
				}
				else if (!m_options.input_precise_mouse)
				{
					// Check if mouse cursor changed cell location
					float cell_width = m_vars[VK_CELL_WIDTH];
					float cell_height = m_vars[VK_CELL_HEIGHT];
					int mx = (int)std::floor(stroke.x/cell_width);
					int my = (int)std::floor(stroke.y/cell_height);
					if (mx == m_vars[VK_MOUSE_X] && my == m_vars[VK_MOUSE_Y])
					{
						must_be_consumed = true;
					}
				}
			}
			else if (stroke.scancode == VK_MOUSE_SCROLL)
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
		if (stroke.scancode == VK_MOUSE_MOVE)
		{
			// Mouse movement event, update mouse position
			float cell_width = m_vars[VK_CELL_WIDTH];
			float cell_height = m_vars[VK_CELL_HEIGHT];
			m_vars[VK_MOUSE_X] = (int)std::floor(stroke.x/cell_width);
			m_vars[VK_MOUSE_Y] = (int)std::floor(stroke.y/cell_height);
			m_vars[VK_MOUSE_PIXEL_X] = stroke.x;
			m_vars[VK_MOUSE_PIXEL_Y] = stroke.y;
		}
		else if (stroke.scancode == VK_MOUSE_SCROLL)
		{
			// Mouse scroll event, update wheel position
			m_vars[VK_MOUSE_WHEEL] = stroke.z; // TODO: relative
		}
		else if (stroke.scancode == VK_CLOSE)
		{
			if (m_options.input_sticky_close || !(m_options.input_events & InputEvents::KeyPress))
			{
				m_vars[VK_CLOSE] = 1;
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
		for ( int i = 0; i < VK_F12; i++ ) // TODO: VK_F12 --> some meaningful name
		{
			if (i == VK_CLOSE) continue;
			if (m_vars[i]) OnWindowInput(Keystroke(i, true));
		}
	}
}
