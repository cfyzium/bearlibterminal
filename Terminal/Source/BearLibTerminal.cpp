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

#define BEARLIBTERMINAL_BUILDING_LIBRARY
#include "BearLibTerminal.h"
#include "Config.hpp"
#include "Terminal.hpp"
#include "Palette.hpp"
#include "Config.hpp"
#include "Log.hpp"
#include <map>
#include <memory>
#include <string>
#include <string.h>
#include <iostream>
#include <thread>

using namespace BearLibTerminal;

namespace
{
	struct cached_setting_t
	{
		cached_setting_t();
		cached_setting_t(std::wstring s);
		cached_setting_t& operator=(std::wstring s);
		template<typename T> const T* get();

		std::string s8;
		std::u16string s16;
		std::u32string s32;
	};

	cached_setting_t::cached_setting_t()
	{ }

	cached_setting_t::cached_setting_t(std::wstring s)
	{
		this->operator=(s);
	}

	cached_setting_t& cached_setting_t::operator=(std::wstring s)
	{
		s8 = BearLibTerminal::UTF8Encoding().Convert(s);
		s16 = BearLibTerminal::UCS2Encoding().Convert(s);
		s32 = BearLibTerminal::UCS4Encoding().Convert(s);
		return *this;
	}

	template<> const char* cached_setting_t::get<char>()
	{
		return s8.c_str();
	}

	template<> const char16_t* cached_setting_t::get<char16_t>()
	{
		return s16.c_str();
	}

	template<> const char32_t* cached_setting_t::get<char32_t>()
	{
		return s32.c_str();
	}

	std::map<std::wstring, cached_setting_t> g_cached_settings;
}

int terminal_open()
{
	if (g_instance)
	{
		LOG(Error, "terminal_open: BearLibTerminal instance already initialized");
		return 0;
	}

	Config::Instance().Reload();
	Config::Instance().TryGet(L"ini.bearlibterminal.log.file", Log::Instance().filename);
	Config::Instance().TryGet(L"ini.bearlibterminal.log.level", Log::Instance().level);
	Config::Instance().TryGet(L"ini.bearlibterminal.log.mode", Log::Instance().mode);

	try
	{
		g_instance.reset(new BearLibTerminal::Terminal());
		return 1;
	}
	catch (std::exception& e)
	{
		LOG(Fatal, "terminal_open: " << e.what());
		return 0;
	}
}

void terminal_close()
{
	if (g_instance)
	{
		g_instance.reset();
		BearLibTerminal::Log::Instance().Reset();
	}
}

int terminal_set8(const int8_t* value)
{
	if (!g_instance || !value) return -1;
	auto& encoding = g_instance->GetEncoding();
	return g_instance->SetOptions(encoding.Convert((const char*)value));
}

int terminal_set16(const int16_t* value)
{
	if (!g_instance || !value) return -1;
	return g_instance->SetOptions(BearLibTerminal::UCS2Encoding().Convert((const char16_t*)value));
}

int terminal_set32(const int32_t* value)
{
	if (!g_instance || !value) return -1;
	return g_instance->SetOptions(BearLibTerminal::UCS4Encoding().Convert((const char32_t*)value));
}

void terminal_refresh()
{
	if (!g_instance) return;
	g_instance->Refresh();
}

void terminal_clear()
{
	if (!g_instance) return;
	g_instance->Clear();
}

void terminal_clear_area(int x, int y, int w, int h)
{
	if (!g_instance) return;
	g_instance->Clear(x, y, w, h);
}

void terminal_crop(int x, int y, int w, int h)
{
	if (!g_instance) return;
	g_instance->SetCrop(x, y, w, h);
}

void terminal_layer(int index)
{
	if (!g_instance) return;
	g_instance->SetLayer(index);
}

void terminal_color(color_t color)
{
	if (!g_instance) return;
	g_instance->SetForeColor(BearLibTerminal::Color(color));
}

void terminal_bkcolor(color_t color)
{
	if (!g_instance) return;
	g_instance->SetBackColor(BearLibTerminal::Color(color));
}

void terminal_composition(int mode)
{
	if (!g_instance) return;
	g_instance->SetComposition(mode);
}

void terminal_font8(const int8_t* name)
{
	if (g_instance)
		g_instance->SetFont(g_instance->GetEncoding().Convert((const char*)name));
}

void terminal_font16(const int16_t* name)
{
	if (g_instance)
		g_instance->SetFont(UCS2Encoding().Convert((const char16_t*)name));
}

void terminal_font32(const int32_t* name)
{
	if (g_instance)
		g_instance->SetFont(UCS4Encoding().Convert((const char32_t*)name));
}

void terminal_put(int x, int y, int code)
{
	if (!g_instance) return;
	g_instance->Put(x, y, code);
}

void terminal_put_ext(int x, int y, int dx, int dy, int code, color_t* corners)
{
	if (!g_instance) return;
	g_instance->PutExtended(x, y, dx, dy, code, (BearLibTerminal::Color*)corners);
}

int terminal_pick(int x, int y, int index)
{
	if (!g_instance) return 0;
	return g_instance->Pick(x, y, index);
}

color_t terminal_pick_color(int x, int y, int index)
{
	if (!g_instance) return 0;
	return g_instance->PickForeColor(x, y, index);
}

color_t terminal_pick_bkcolor(int x, int y)
{
	if (!g_instance) return 0;
	return g_instance->PickBackColor(x, y);
}

#define TERMINAL_PRINT_OR_MEASURE(x, y, a, cast, measure) \
	if (!g_instance || !s) { \
		if (out_w) *out_w = 0; \
		if (out_h) *out_h = 0; \
		return; \
	} \
	auto size = g_instance->Print(x, y, w, h, a, cast, false, measure); \
	if (out_w) *out_w = size.width; \
	if (out_h) *out_h = size.height;

void terminal_print_ext8(int x, int y, int w, int h, int align, const int8_t* s, int* out_w, int* out_h)
{
	TERMINAL_PRINT_OR_MEASURE(x, y, align, g_instance->GetEncoding().Convert((const char*)s), false)
}

void terminal_print_ext16(int x, int y, int w, int h, int align, const int16_t* s, int* out_w, int* out_h)
{
	TERMINAL_PRINT_OR_MEASURE(x, y, align, BearLibTerminal::UCS2Encoding().Convert((const char16_t*)s), false)
}

void terminal_print_ext32(int x, int y, int w, int h, int align, const int32_t* s, int* out_w, int* out_h)
{
	TERMINAL_PRINT_OR_MEASURE(x, y, align, BearLibTerminal::UCS4Encoding().Convert((const char32_t*)s), false)
}

void terminal_measure_ext8(int w, int h, const int8_t* s, int* out_w, int* out_h)
{
	TERMINAL_PRINT_OR_MEASURE(0, 0, TK_ALIGN_DEFAULT, g_instance->GetEncoding().Convert((const char*)s), true)
}

void terminal_measure_ext16(int w, int h, const int16_t* s, int* out_w, int* out_h)
{
	TERMINAL_PRINT_OR_MEASURE(0, 0, TK_ALIGN_DEFAULT, BearLibTerminal::UCS2Encoding().Convert((const char16_t*)s), true)
}

void terminal_measure_ext32(int w, int h, const int32_t* s, int* out_w, int* out_h)
{
	TERMINAL_PRINT_OR_MEASURE(0, 0, TK_ALIGN_DEFAULT, BearLibTerminal::UCS4Encoding().Convert((const char32_t*)s), true)
}

int terminal_has_input()
{
	if (!g_instance) return 1;
	return g_instance->HasInput();
}

int terminal_state(int code)
{
	if (!g_instance) return 0;
	return g_instance->GetState(code);
}

int terminal_read()
{
	if (!g_instance) return TK_CLOSE;
	return g_instance->Read();
}

template<typename char_t, typename enc_t> int read_str(int x, int y, char_t* buffer, int max, const enc_t& encoding)
{
	if (!g_instance) return -1;
	std::wstring wide_buffer = encoding.Convert((const char_t*)buffer);
	wide_buffer.reserve(max+1);
	int rc = g_instance->ReadString(x, y, &wide_buffer[0], max);
	if (rc >= 0)
	{
		std::basic_string<char_t> result = encoding.Convert(wide_buffer.c_str());
		memcpy(buffer, result.data(), sizeof(char_t)*(result.length()+1));
	}
	return rc;
}

int terminal_read_str8(int x, int y, int8_t* buffer, int max)
{
	if (!g_instance) return TK_INPUT_CANCELLED;
	return read_str(x, y, (char*)buffer, max, g_instance->GetEncoding());
}

int terminal_read_str16(int x, int y, int16_t* buffer, int max)
{
	if (!g_instance) return TK_INPUT_CANCELLED;
	return read_str(x, y, (char16_t*)buffer, max, BearLibTerminal::UCS2Encoding());
}

int terminal_read_str32(int x, int y, int32_t* buffer, int max)
{
	if (!g_instance) return TK_INPUT_CANCELLED;
	return read_str(x, y, (char32_t*)buffer, max, BearLibTerminal::UCS4Encoding());
}

int terminal_peek()
{
	if (!g_instance) return TK_CLOSE;
	return g_instance->Peek();
}

void terminal_delay(int period)
{
	if (g_instance)
		g_instance->Delay(period);
	else
		std::this_thread::sleep_for(std::chrono::milliseconds{period});
}

template<typename outer, typename inner> const outer* terminal_get(const outer* key, const outer* default_)
{
	typename BearLibTerminal::Encodings<inner>::type encoding;
	std::wstring wkey = encoding.Convert(std::basic_string<inner>((const inner*)key));
	std::wstring wout;
	if (!BearLibTerminal::Config::Instance().TryGet(wkey, wout))
	{
		wout = default_ == nullptr? std::wstring(L""): encoding.Convert(std::basic_string<inner>((const inner*)default_));
	}
	auto& cached_setting = g_cached_settings[wkey];
	cached_setting = wout;
	return (const outer*)cached_setting.get<inner>();
}

const int8_t* terminal_get8(const int8_t* key, const int8_t* default_)
{
	return terminal_get<int8_t, char>(key, default_);
}

const int16_t* terminal_get16(const int16_t* key, const int16_t* default_)
{
	return terminal_get<int16_t, char16_t>(key, default_);
}

const int32_t* terminal_get32(const int32_t* key, const int32_t* default_)
{
	return terminal_get<int32_t, char32_t>(key, default_);
}

color_t color_from_name8(const int8_t* name)
{
	if (!g_instance || !name) return -1;
	auto& encoding = g_instance->GetEncoding();
	return BearLibTerminal::Palette::Instance.Get(encoding.Convert((const char*)name));
}

color_t color_from_name16(const int16_t* name)
{
	if (!g_instance || !name) return -1;
	return BearLibTerminal::Palette::Instance.Get(BearLibTerminal::UCS2Encoding().Convert((const char16_t*)name));
}

color_t color_from_name32(const int32_t* name)
{
	if (!g_instance || !name) return -1;
	return BearLibTerminal::Palette::Instance.Get(BearLibTerminal::UCS4Encoding().Convert((const char32_t*)name));
}

int terminal_put_array(int x, int y, int w, int h, const uint8_t* data, int row_stride, int column_stride, const void* layout, int char_size)
{
	if (!g_instance)
	{
		return -1;
	}

	std::wstring s;
	switch (char_size)
	{
	case 1:
		s = g_instance->GetEncoding().Convert((const char*)layout);
		break;
	case 2:
		s = BearLibTerminal::UCS2Encoding().Convert((const char16_t*)layout);
		break;
	case 4:
		s = BearLibTerminal::UCS4Encoding().Convert((const char32_t*)layout);
		break;
	default:
		return -1;
	}

	return g_instance->PutArray(x, y, w, h, data, row_stride, column_stride, s);
}
