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

#include "OpenGL.hpp"
#include "Log.hpp"
#include <string>
#include <algorithm>

namespace BearLibTerminal
{
	int g_max_texture_size = 256;
	bool g_has_texture_npot = false;

	void ProbeOpenGL()
	{
		GLint size;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
		g_max_texture_size = size;
		LOG(Info, "OpenGL: maximum texture size is " << size << "x" << size);

		std::string extensions = (const char*)glGetString(GL_EXTENSIONS);
		std::transform(extensions.begin(), extensions.end(), extensions.begin(), ::tolower);
		g_has_texture_npot = extensions.find("gl_arb_texture_non_power_of_two") != std::string::npos;
		LOG(Info, "OpenGL: GPU " << (g_has_texture_npot? "supports": "does not support") << " NPOTD textures");
	}
}
