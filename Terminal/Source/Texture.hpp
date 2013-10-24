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

#ifndef BEARLIBTERMINAL_TEXTURE_HPP
#define BEARLIBTERMINAL_TEXTURE_HPP

#include <cstdint>
#include "Bitmap.hpp"
#include "Size.hpp"

namespace BearLibTerminal
{
	class Texture
	{
	public:
		typedef std::uint32_t handle_t;

		Texture();
		Texture(const Bitmap& bitmap);
		virtual ~Texture();
		void Dispose();
		void Bind();
		void Update(const Bitmap& bitmap);
		void Update(Rectangle area, const Bitmap& bitmap);
		Size GetSize() const;
		static void Enable();
		static void Disable();
		static void Unbind();

		// NonCopyable, NonAssignable
		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;

	protected:
		handle_t m_handle;
		Size m_size;
		static handle_t m_currently_bound_handle;
	};
}

#endif // BEARLIBTERMINAL_TEXTURE_HPP
