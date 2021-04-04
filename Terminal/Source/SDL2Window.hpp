/*
* BearLibTerminal
* Copyright (C) 2013-2021 Cfyz
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

#ifndef BEARLIBTERMINAL_SDL2WINDOW_HPP_
#define BEARLIBTERMINAL_SDL2WINDOW_HPP_

#include "Window.hpp"
#include <SDL2/SDL.h>

namespace BearLibTerminal {

class SDL2Window: public Window
{
public:
	SDL2Window(EventHandler handler);
	~SDL2Window();
	virtual Size GetActualSize() override;
	virtual Size GetDrawableSize() override;
	virtual std::wstring GetClipboard() override;
	virtual void SetTitle(const std::wstring& title) override;
	virtual void SetIcon(const std::wstring& filename) override;
	virtual void SetSizeHints(Size increment, Size minimum_size) override;
	virtual void SetClientSize(const Size& size) override;
	virtual void Show() override;
	virtual void Hide() override;
	virtual void SwapBuffers() override;
	virtual void SetVSync(bool enabled) override;
	virtual void SetResizeable(bool resizeable) override;
	virtual void SetFullscreen(bool fullscreen) override;
	virtual void SetCursorVisibility(bool visible) override;
	virtual int PumpEvents() override;

private:
	void Create(); // XXX: shadows static Create()
	void Dispose();

private:
	SDL_Window* m_Window;
	SDL_GLContext m_GL_Context;
};

} // namespace BearLibTerminal

#endif // BEARLIBTERMINAL_SDL2WINDOW_HPP_
