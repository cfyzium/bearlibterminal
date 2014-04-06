/*
* BearLibTerminal
* Copyright (C) 2014 Cfyz
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

#include "SDL2Window.hpp"
#include "Encoding.hpp"
#include "Platform.hpp"
#include "Utility.hpp"
#include <stdint.h>

namespace BearLibTerminal
{
	namespace
	{
		typedef struct SDL_Window SDL_Window;

		typedef struct SDL_Renderer SDL_Renderer;

		typedef struct SDL_RendererInfo
		{
		    const char *name;             // The name of the renderer
		    uint32_t flags;               // Supported ::SDL_RendererFlags
		    uint32_t num_texture_formats; // The number of available texture formats
		    uint32_t texture_formats[16]; // The available texture formats
		    int max_texture_width;        // The maximimum texture width
		    int max_texture_height;       // The maximimum texture height
		} SDL_RendererInfo;

		typedef struct SDL_Surface SDL_Surface;

		typedef const char* (*PFNSDLGETERROR)(void);
		typedef int (*PFNSDLINIT)(uint32_t flags);
		typedef SDL_Window* (*PFNSDLCREATEWINDOW)(const char *title, int x, int y, int w, int h, uint32_t flags);
		typedef SDL_Renderer* (*PFNSDLCREATERENDERER)(SDL_Window* window, int index, uint32_t flags);
		typedef int (*PFNSDLGETRENDERERINFO)(SDL_Renderer* renderer, SDL_RendererInfo* info);
		typedef void (*PFNSDLRENDERPRESENT)(SDL_Renderer* renderer);
		typedef void (*PFNSDLSETWINDOWICON)(SDL_Window* window, SDL_Surface* icon);
		typedef void (*PFNSDLSETWINDOWMINIMUMSIZE)(SDL_Window* window, int min_w, int min_h);
		typedef void (*PFNSDLSETWINDOWSIZE)(SDL_Window* window, int w, int h);
		typedef void (*PFNSDLSETWINDOWTITLE)(SDL_Window* window, const char* title);
		typedef void (*PFNSDLSHOWWINDOW)(SDL_Window* window);
		typedef void (*PFNSDLHIDEWINDOW)(SDL_Window* window);
		typedef void (*PFNSDLDESTROYRENDERER)(SDL_Renderer* renderer);
		typedef void (*PFNSDLDESTROYWINDOW)(SDL_Window* window);
		typedef SDL_Surface* (*PFNSDLCREATERGBSURFACEFROM)(void* pixels, int width, int height, int depth, int pitch, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask);
		typedef void (*PFNSDLFREESURFACE)(SDL_Surface* surface);
	}

	struct SDL2Window::Private
	{
		Private();

		Module libSDL2;
		PFNSDLGETERROR SDL_GetError;
		PFNSDLINIT SDL_Init;
		PFNSDLCREATEWINDOW SDL_CreateWindow;
		PFNSDLCREATERENDERER SDL_CreateRenderer;
		PFNSDLGETRENDERERINFO SDL_GetRendererInfo;
		PFNSDLRENDERPRESENT SDL_RenderPresent;
		PFNSDLSETWINDOWICON SDL_SetWindowIcon;
		PFNSDLSETWINDOWMINIMUMSIZE SDL_SetWindowMinimumSize;
		PFNSDLSETWINDOWSIZE SDL_SetWindowSize;
		PFNSDLSETWINDOWTITLE SDL_SetWindowTitle;
		PFNSDLSHOWWINDOW SDL_ShowWindow;
		PFNSDLHIDEWINDOW SDL_HideWindow;
		PFNSDLDESTROYRENDERER SDL_DestroyRenderer;
		PFNSDLDESTROYWINDOW SDL_DestroyWindow;
		PFNSDLCREATERGBSURFACEFROM SDL_CreateRGBSurfaceFrom;
		PFNSDLFREESURFACE SDL_FreeSurface;

		SDL_Window* window;
		SDL_Renderer* renderer;
	};

	SDL2Window::Private::Private()
	{
		libSDL2 = Module(L"libSDL2-2.0.so.0"); // TODO: soname compatibility

		SDL_GetError = (PFNSDLGETERROR)libSDL2["SDL_GetError"];
		SDL_Init = (PFNSDLINIT)libSDL2["SDL_Init"];
		SDL_CreateWindow = (PFNSDLCREATEWINDOW )libSDL2["SDL_CreateWindow"];
		SDL_CreateRenderer = (PFNSDLCREATERENDERER)libSDL2["SDL_CreateRenderer"];
		SDL_GetRendererInfo = (PFNSDLGETRENDERERINFO)libSDL2["SDL_"];
		SDL_RenderPresent = (PFNSDLRENDERPRESENT)libSDL2["SDL_RenderPresent"];
		SDL_SetWindowIcon = (PFNSDLSETWINDOWICON)libSDL2["SDL_SetWindowIcon"];
		SDL_SetWindowMinimumSize = (PFNSDLSETWINDOWMINIMUMSIZE)libSDL2["SDL_SetWindowMinimumSize"];
		SDL_SetWindowSize = (PFNSDLSETWINDOWSIZE)libSDL2["SDL_SetWindowSize"];
		SDL_SetWindowTitle = (PFNSDLSETWINDOWTITLE)libSDL2["SDL_SetWindowTitle"];
		SDL_ShowWindow = (PFNSDLSHOWWINDOW)libSDL2["SDL_ShowWindow"];
		SDL_HideWindow = (PFNSDLHIDEWINDOW)libSDL2["SDL_HideWindow"];
		SDL_DestroyRenderer = (PFNSDLDESTROYRENDERER)libSDL2["SDL_DestroyRenderer"];
		SDL_DestroyWindow = (PFNSDLDESTROYWINDOW)libSDL2["SDL_DestroyWindow"];
		SDL_CreateRGBSurfaceFrom = (PFNSDLCREATERGBSURFACEFROM)libSDL2["SDL_CreateRGBSurfaceFrom"];
		SDL_FreeSurface = (PFNSDLFREESURFACE)libSDL2["SDL_FreeSurface"];

		window = nullptr;
		renderer = nullptr;
	}

	SDL2Window::SDL2Window()
	{
		try
		{
			m_private = std::make_unique<Private>();
		}
		catch (std::exception& e)
		{
			LOG(Fatal, "Loading SDL2 failed: " << e.what());
			throw std::runtime_error("SDL2Window construction failed");
		}
	}

	SDL2Window::~SDL2Window()
	{ }

	bool SDL2Window::ValidateIcon(const std::wstring& filename)
	{
		return true; // FIXME: NYI
	}

	void SDL2Window::SetTitle(const std::wstring& title)
	{
		if (m_private->window == nullptr) return;
		m_private->SDL_SetWindowTitle(m_private->window, UTF8->Convert(title).c_str());
	}

	void SDL2Window::SetIcon(const std::wstring& filename)
	{
		// FIXME: NYI
	}

	void SDL2Window::SetClientSize(const Size& size)
	{
		if (m_private->window == nullptr) return;
		m_private->SDL_SetWindowSize(m_private->window, size.width, size.height);
		// TODO: update minimum size here
	}

	void SDL2Window::Redraw()
	{
		// FIXME: NYI
	}

	void SDL2Window::Show()
	{
		if (m_private->window == nullptr) return;
		m_private->SDL_ShowWindow(m_private->window);
	}

	void SDL2Window::Hide()
	{
		if (m_private->window == nullptr) return;
		m_private->SDL_HideWindow(m_private->window);
	}

	void SDL2Window::Invoke(std::function<void()> func)
	{
		// FIXME: NYI
	}

	void SDL2Window::SwapBuffers()
	{
		if (m_private->window == nullptr || m_private->renderer == nullptr) return;
		m_private->SDL_RenderPresent(m_private->renderer);
	}

	void SDL2Window::SetVSync(bool enabled)
	{
		// FIXME: NYI
	}

	bool SDL2Window::PumpEvents()
	{
		// FIXME: NYI
	}

	void SDL2Window::SetResizeable(bool resizeable)
	{
		// FIXME: NYI
		// TODO: update minimum size here
	}

	void SDL2Window::ThreadFunction()
	{
		// FIXME: NYI
	}

	bool SDL2Window::Construct()
	{
		// FIXME: NYI
	}

	void SDL2Window::Destroy()
	{
		// FIXME: NYI
	}
}
