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

#include <stdexcept>
#include "Texture.hpp"
//#include "Utility.hpp"
#include "OpenGL.hpp"
#include "Log.hpp"

namespace BearLibTerminal
{
	static const GLenum color_format = GL_BGRA;

	//Texture::handle_t Texture::m_currently_bound_handle = 0;
	std::atomic<uint32_t> Texture::m_currently_bound_handle{0};

	static bool IsPowerOfTwo(int value)
	{
		return (value != 0) && !(value & (value-1));
	}

	Texture::Texture():
		m_handle(0)
	{ }

	Texture::Texture(const Bitmap& bitmap):
		m_handle(0)
	{
		Update(bitmap);
	}

	Texture::~Texture()
	{
		Dispose();
	}

	void Texture::Dispose()
	{
		if (m_handle > 0)
		{
			Unbind();
			glDeleteTextures(1, &m_handle);
			m_handle = 0;
		}
	}

	void Texture::Bind()
	{
		if (m_handle == 0)
		{
			LOG(Error, L"[Texture::Bind] texture is not allocated yet");
			throw std::runtime_error("invalid texture handle");
		}

		if (m_handle != m_currently_bound_handle)
		{
			glBindTexture(GL_TEXTURE_2D, m_handle);
			m_currently_bound_handle = m_handle;
		}
	}

	void Texture::Update(const Bitmap& bitmap)
	{
		// This class implements POTD texture only
		Size bitmap_size = bitmap.GetSize();
		if ((!IsPowerOfTwo(bitmap_size.width) || !IsPowerOfTwo(bitmap_size.height)) && !g_has_texture_npot)
		{
			LOG(Error, L"[Texture::Update] supplied bitmap is NPOTD");
			throw std::runtime_error("invalid bitmap");
		}

		/*
		// Clear OpenGL error stack
		while ( glGetError() != GL_NO_ERROR );
		//*/

		if (m_handle == 0)
		{
			// Allocate a fresh texture object
			m_size = bitmap_size;
			glGenTextures(1, &m_handle);
			Bind();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_size.width, m_size.height, 0, color_format, GL_UNSIGNED_BYTE, (uint8_t*)bitmap.GetData());
		}
		else
		{
			// Texture object already exists, update
			Bind();
			if (bitmap_size == m_size)
			{
				// Texture may be updated in-place
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_size.width, m_size.height, color_format, GL_UNSIGNED_BYTE, (uint8_t*)bitmap.GetData());
			}
			else
			{
				// Texture must be reallocated with new size (done by driver)
				m_size = bitmap_size;
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_size.width, m_size.height, 0, color_format, GL_UNSIGNED_BYTE, (uint8_t*)bitmap.GetData());
			}
		}

		/*
		bool error_occured = false;
		GLenum error_code = GL_NO_ERROR;
		while ( (error_code = glGetError()) != GL_NO_ERROR )
		{
			// TODO: Log
		}

		if ( error_occured )
		{
			throw std::runtime_error("Texture::Update: OpenGL error");
		}
		//*/
	}

	Bitmap Texture::Download()
	{
		if (m_handle == 0)
		{
			LOG(Error, L"[Texture::Download] Texture is not yet created");
			throw std::runtime_error("invalid texture");
		}

		Bitmap result(m_size, Color());
		uint8_t* data = (uint8_t*)&result(0, 0);

		Bind();
		glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);

		return result;
	}

	void Texture::Update(Rectangle area, const Bitmap& bitmap)
	{
		if (m_handle == 0)
		{
			throw std::runtime_error("Texture::Update(Rectangle, const Bitmap&): uninitialized texture");
		}

		if (area.Size() != bitmap.GetSize() || !Rectangle(m_size).Contains(area))
		{
			throw std::runtime_error("Texture::Update(Rectangle, const Bitmap&): invalid area");
		}

		Bind();
		glTexSubImage2D(GL_TEXTURE_2D, 0, area.left, area.top, area.width, area.height, color_format, GL_UNSIGNED_BYTE, (uint8_t*)bitmap.GetData());
	}

	Size Texture::GetSize() const
	{
		return m_size;
	}

	Texture::handle_t Texture::GetHandle() const
	{
		return m_handle;
	}

	void Texture::Enable()
	{
		glEnable(GL_TEXTURE_2D);
	}

	void Texture::Disable()
	{
		glDisable(GL_TEXTURE_2D);
	}

	void Texture::Unbind()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		m_currently_bound_handle = 0;
	}

	Texture::handle_t Texture::BoundId()
	{
		return m_currently_bound_handle;
	}
}
