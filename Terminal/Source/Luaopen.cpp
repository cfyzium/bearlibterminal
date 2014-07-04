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

#define BEARLIBTERMINAL_BUILDING_LIBRARY
#include "BearLibTerminal.h"
#include "Platform.hpp"
#include "Log.hpp"
#include <vector>
#include <type_traits>
#include <stddef.h>
#include <string.h>

typedef struct lua_State lua_State;
typedef double lua_Number;
typedef int (*lua_CFunction) (lua_State *L);

struct luaL_Reg
{
	const char *name;
	lua_CFunction func;
};

// Types
#define LUA_TNONE               (-1)
#define LUA_TNIL                0
#define LUA_TBOOLEAN            1
#define LUA_TLIGHTUSERDATA      2
#define LUA_TNUMBER             3
#define LUA_TSTRING             4
#define LUA_TTABLE              5
#define LUA_TFUNCTION           6
#define LUA_TUSERDATA           7
#define LUA_TTHREAD             8

#define LUA_INTEGER             ptrdiff_t
typedef LUA_INTEGER             lua_Integer;

extern "C"
{
	TERMINAL_API int luaopen_BearLibTerminal(lua_State* L);
}

typedef int (*PFNLUAGETTOP)(lua_State* L); // lua_gettop
typedef void (*PFNLUACREATETABLE)(lua_State* L, int narr, int nrec); // lua_createtable
typedef void (*PFNLUALCHECKSTACK)(lua_State* L, int space, const char* msg); // luaL_checkstack
typedef void (*PFNLUAPUSHVALUE)(lua_State *L, int idx); // lua_pushvalue
typedef void (*PFNLUAPUSHCCLOSURE)(lua_State *L, lua_CFunction fn, int n); // lua_pushcclosure
typedef void (*PFNLUASETFIELD)(lua_State *L, int idx, const char *k); // lua_setfield
typedef void (*PFNLUASETTOP)(lua_State *L, int idx); // lua_settop
typedef void (*PFNLUAPUSHNUMBER) (lua_State *L, lua_Number n); // lua_pushnumber
typedef void (*PFNLUAPUSHINTEGER) (lua_State *L, lua_Integer n); // lua_pushinteger
typedef const char* (*PFNLUATOSTRING)(lua_State *L, int idx, size_t *len); // lua_tolstring
typedef lua_Number (*PFNLUATONUMBER)(lua_State *L, int idx); // lua_tonumber
typedef lua_Number (*PFNLUATONUMBERX)(lua_State *L, int index, int* isnum); // lua_tonumberx
typedef lua_Integer (*PFNLUATOINTEGER)(lua_State *L, int idx); // lua_tointeger
typedef lua_Integer (*PFNLUATOINTEGERX)(lua_State *L, int idx, int* isnum); // lua_tointegerx
typedef void (*PFNLUARAWGETI)(lua_State *L, int idx, int n); // lua_rawgeti
typedef int (*PFNLUATYPE)(lua_State *L, int idx); // lua_type
typedef void (*PFNLUAPUSHBOOLEAN)(lua_State *L, int b); // lua_pushboolean
typedef void (*PFNLUAGETTABLE)(lua_State *L, int idx); // lua_gettable
typedef const char* (*PFNLUAPUSHSTRING)(lua_State *L, const char *s); // lua_pushstring
typedef int (*PFNLUAPCALL)(lua_State *L, int nargs, int nresults, int errfunc); // lua_pcall
typedef int (*PFNLUAPCALLK)(lua_State *L, int nargs, int nresults, int errfunc, int ctx, lua_CFunction k); // lua_pcallk
typedef void (*PFNLUAGETFIELD)(lua_State *L, int idx, const char *k); // lua_getfield
typedef void (*PFNLUAINSERT)(lua_State *L, int idx); // lua_insert
typedef void (*PFNLUAREPLACE)(lua_State *L, int index); // lua_replace
typedef int (*PFNLUAERROR)(lua_State *L); // lua_error
typedef size_t (*PFNLUAOBJLEN)(lua_State *L, int index); // lua_objlen
typedef size_t (*PFNLUARAWLEN)(lua_State *L, int idx); // lua_rawlen

static PFNLUAGETTOP lua_gettop = 0;
static PFNLUACREATETABLE lua_createtable = 0;
static PFNLUALCHECKSTACK luaL_checkstack = 0;
static PFNLUAPUSHVALUE lua_pushvalue = 0;
static PFNLUAPUSHCCLOSURE lua_pushcclosure = 0;
static PFNLUASETFIELD lua_setfield = 0;
static PFNLUASETTOP lua_settop = 0;
static PFNLUAPUSHNUMBER lua_pushnumber = 0;
static PFNLUAPUSHINTEGER lua_pushinteger = 0;
static PFNLUATOSTRING lua_tolstring = 0;
static PFNLUATONUMBER lua_tonumber = 0;
static PFNLUATONUMBERX lua_tonumberx = 0;
static PFNLUATOINTEGER lua_tointeger = 0;
static PFNLUATOINTEGERX lua_tointegerx = 0;
static PFNLUARAWGETI lua_rawgeti = 0;
static PFNLUATYPE lua_type = 0;
static PFNLUAPUSHBOOLEAN lua_pushboolean = 0;
static PFNLUAGETTABLE lua_gettable = 0;
static PFNLUAPUSHSTRING lua_pushstring = 0;
static PFNLUAPCALL lua_pcall = 0;
static PFNLUAPCALLK lua_pcallk = 0;
static PFNLUAGETFIELD lua_getfield = 0;
static PFNLUAINSERT lua_insert = 0;
static PFNLUAREPLACE lua_replace = 0;
static PFNLUAERROR lua_error = 0;
static PFNLUAOBJLEN lua_objlen = 0;
static PFNLUARAWLEN lua_rawlen = 0;

#define lua_pop(L,n)			lua_settop(L, -(n)-1)
#define luaL_newlibtable(L,l)	lua_createtable(L, 0, sizeof(l)/sizeof((l)[0]) - 1)
#define luaL_newlib(L,l)		(luaL_newlibtable(L,l), luaL_setfuncs(L,l,0))
#define lua_tostring(L,i)		lua_tolstring(L, (i), NULL)

lua_Number lua_tonumber_52(lua_State* L, int index)
{
	return lua_tonumberx(L, index, nullptr);
}

lua_Integer lua_tointeger_52(lua_State* L, int index)
{
	return lua_tointegerx(L, index, nullptr);
}

int lua_pcall_52(lua_State *L, int nargs, int nresults, int errfunc)
{
	return lua_pcallk(L, nargs, nresults, errfunc, 0, nullptr);
}

size_t lua_objlen_52(lua_State* L, int index)
{
	return lua_rawlen(L, index);
}

void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup)
{
	luaL_checkstack(L, nup, "too many upvalues");
	for (; l->name != NULL; l++)
	{
		// fill the table with given functions
		for (int i = 0; i < nup; i++)
		{
			// copy upvalues to the top
			lua_pushvalue(L, -nup);
		}
		lua_pushcclosure(L, l->func, nup); // closure with those upvalues
		lua_setfield(L, -(nup + 2), l->name);
	}
	// remove upvalues
	lua_pop(L, nup);
}

int luaterminal_open(lua_State* L)
{
	lua_pushboolean(L, terminal_open());
	return 1;
}

int luaterminal_close(lua_State* L)
{
	terminal_close();
	return 0;
}

int luaterminal_set(lua_State* L)
{
	const char* s = lua_tostring(L, 1);
	lua_pushboolean(L, terminal_set8((const int8_t*)s));
	return 1;
}

int luaterminal_setf(lua_State* L)
{
	// Stack: [s, arg1, arg2, arg3, ...]
	int nargs = lua_gettop(L);
	if (nargs < 1) // Because Lua won't be as lenient with argument type errors here
	{
		lua_pushstring(L, "luaterminal_setf: not enough arguments");
		lua_error(L);
		return 0;
	}
	else if (lua_type(L, 1) != LUA_TSTRING)
	{
		lua_pushstring(L, "luaterminal_setf: first argument is not a string");
		lua_error(L);
		return 0;
	}

	lua_getfield(L, 1, "format"); // Gets the "format" field from the first (bottom) stack element which is string
	lua_insert(L, 1); // Shift retrieved function to the bottom of the stack

	// Special case: replace tables with addresses of cached bitmaps
	std::vector<std::vector<color_t>> cached;
	// Now stack looks like [format, s, arg1, arg2, arg3, ...]
	for (int i=3; i<=nargs+1; i++)
	{
		if (lua_type(L, i) == LUA_TTABLE)
		{
			// Cache
			size_t size = lua_objlen(L, i);
			cached.emplace_back(size, 0);
			for (int j=0; j<size; j++)
			{
				lua_pushinteger(L, j+1);
				lua_gettable(L, i);
				cached.back()[j] = lua_tonumber(L, -1);
				lua_pop(L, 1);
			}

			// Replace with address
			std::ostringstream ss;
			ss << "0x" << std::hex << (uint64_t)cached.back().data();
			lua_pushstring(L, ss.str().c_str());
			lua_replace(L, i);
		}
	}

	lua_pcall(L, nargs, 1, 0);

	const char* s = lua_tostring(L, 1);
	lua_pushboolean(L, terminal_set8((const int8_t*)s));
	return 1;
}

int luaterminal_refresh(lua_State* L)
{
	terminal_refresh();
	return 0;
}

int luaterminal_clear(lua_State* L)
{
	terminal_clear();
	return 0;
}

int luaterminal_clear_area(lua_State* L)
{
	int x = lua_tointeger(L, 1);
	int y = lua_tointeger(L, 2);
	int w = lua_tointeger(L, 3);
	int h = lua_tointeger(L, 4);
	terminal_clear_area(x, y, w, h);
	return 0;
}

int luaterminal_layer(lua_State* L)
{
	terminal_layer(lua_tointeger(L, 1));
	return 0;
}

int luaterminal_color(lua_State* L)
{
	int type = lua_type(L, 1);

	if (type == LUA_TNUMBER)
	{
		terminal_color(lua_tonumber(L, 1));
	}
	else if (type == LUA_TSTRING)
	{
		terminal_color(color_from_name8((const int8_t*)lua_tostring(L, 1)));
	}
	else
	{
		terminal_color(0xFFFFFFFF);
	}

	return 0;
}

int luaterminal_bkcolor(lua_State* L)
{
	int type = lua_type(L, 1);

	if (type == LUA_TNUMBER)
	{
		terminal_bkcolor(lua_tonumber(L, 1));
	}
	else if (type == LUA_TSTRING)
	{
		terminal_bkcolor(color_from_name8((const int8_t*)lua_tostring(L, 1)));
	}
	else
	{
		terminal_bkcolor((color_t)0);
	}

	return 0;
}

int luaterminal_composition(lua_State* L)
{
	terminal_composition(lua_tointeger(L, 1));
	return 0;
}

int luaterminal_put(lua_State* L)
{
	terminal_put(lua_tointeger(L, 1), lua_tointeger(L, 2), lua_tointeger(L, 3));
	return 0;
}

int luaterminal_put_ext(lua_State* L)
{
	int nargs = lua_gettop(L); // put_ext(x, y, dx, dy, code[, corners]) -- 5, 6 or 9 arguments
	if (nargs < 5) return 0;

	int x = lua_tointeger(L, 1);
	int y = lua_tointeger(L, 2);
	int dx = lua_tointeger(L, 3);
	int dy = lua_tointeger(L, 4);
	int code = lua_tointeger(L, 5);

	if (nargs == 5)
	{
		terminal_put_ext(x, y, dx, dy, code, nullptr);
	}
	else if (nargs == 6)
	{
		color_t corners[4];
		for (int i=0; i<4; i++)
		{
			lua_pushinteger(L, i+1);
			lua_gettable(L, -2);
			corners[i] = lua_tonumber(L, -1);
			lua_pop(L, 1);
		}
		terminal_put_ext(x, y, dx, dy, code, corners);
	}
	else if (nargs == 9)
	{
		color_t corners[4] =
		{
			(color_t)lua_tonumber(L, 6),
			(color_t)lua_tonumber(L, 7),
			(color_t)lua_tonumber(L, 8),
			(color_t)lua_tonumber(L, 9)
		};
		terminal_put_ext(x, y, dx, dy, code, corners);
	}

	return 0;
}

int luaterminal_print(lua_State* L)
{
	int rc = terminal_print8(lua_tonumber(L, 1), lua_tonumber(L, 2), (const int8_t*)lua_tostring(L, 3));
	lua_pushnumber(L, rc);
	return 1;
}

int luaterminal_printf(lua_State* L)
{
	// Stack: [x, y, s, arg1, arg2, arg3, ...]
	int nargs = lua_gettop(L);
	if (nargs < 3) // Because Lua won't be as lenient with argument type errors here
	{
		lua_pushstring(L, "luaterminal_printf: not enough arguments");
		lua_error(L);
		return 0;
	}
	else if (lua_type(L, 3) != LUA_TSTRING)
	{
		lua_pushstring(L, "luaterminal_printf: third argument is not a string");
		lua_error(L);
		return 0;
	}

	lua_getfield(L, 3, "format"); // Gets the "format" field from the first (bottom) stack element which is string
	lua_insert(L, 3); // Shift retrieved function to the bottom of the stack

	lua_pcall(L, nargs-2, 1, 0);

	int rc = terminal_print8(lua_tonumber(L, 1), lua_tonumber(L, 2), (const int8_t*)lua_tostring(L, 3));
	lua_pushnumber(L, rc);
	return 1;
}

int luaterminal_has_input(lua_State* L)
{
	lua_pushboolean(L, terminal_has_input());
	return 1;
}

int luaterminal_state(lua_State* L)
{
	lua_pushnumber(L, terminal_state(lua_tonumber(L, 1)));
	return 1;
}

int luaterminal_check(lua_State* L)
{
	lua_pushboolean(L, terminal_state(lua_tonumber(L, 1)) == 1);
	return 1;
}

int luaterminal_read(lua_State* L)
{
	int code = terminal_read();
	int released = (code & TK_KEY_RELEASED) > 0;
	code = code & 0xFF;

	lua_pushnumber(L, code);
	lua_pushboolean(L, released);

	return 2;
}

int luaterminal_read_str(lua_State* L)
{
	// rc, str = terminal.read_str(x, y, str[, max])

	int nargs = lua_gettop(L); // 3 or 4 arguments

	int x = lua_tointeger(L, 1);
	int y = lua_tointeger(L, 2);
	std::string s = lua_tostring(L, 3);
	int max = nargs > 3? lua_tointeger(L, 4): terminal_state(TK_WIDTH)-x;
	std::vector<char> buffer(max*3+1, 0);
	memcpy(buffer.data(), s.data(), std::min(buffer.size()-1, s.size()));

	int rc = terminal_read_str(x, y, buffer.data(), max);
	lua_pushinteger(L, rc);
	lua_pushstring(L, buffer.data());

	return 2;
}

int luaterminal_color_from_name(lua_State* L)
{
	lua_pushnumber(L, color_from_name8((const int8_t*)lua_tostring(L, 1)));
	return 1;
}

int luaterminal_color_from_argb(lua_State* L)
{
	int a = lua_tointeger(L, 1);
	int r = lua_tointeger(L, 2);
	int g = lua_tointeger(L, 3);
	int b = lua_tointeger(L, 4);
	lua_pushnumber(L, color_from_argb(a, r, g, b));
	return 1;
}

static const luaL_Reg luaterminal_lib[] =
{
	{"open", luaterminal_open},
	{"close", luaterminal_close},
	{"set", luaterminal_set},
	{"setf", luaterminal_setf},
	{"refresh", luaterminal_refresh},
	{"clear", luaterminal_clear},
	{"clear_area", luaterminal_clear_area},
	{"layer", luaterminal_layer},
	{"color", luaterminal_color},
	{"bkcolor", luaterminal_bkcolor},
	{"composition", luaterminal_composition},
	{"put", luaterminal_put},
	{"put_ext", luaterminal_put_ext},
	{"print", luaterminal_print},
	{"printf", luaterminal_printf},
	{"has_input", luaterminal_has_input},
	{"state", luaterminal_state},
	{"check", luaterminal_check},
	{"read", luaterminal_read},
	{"read_str", luaterminal_read_str},
	{"color_from_name", luaterminal_color_from_name},
	{"color_from_argb", luaterminal_color_from_argb},
	{NULL, NULL}
};

#define CONST(x) {#x, x}

struct
{
	const char* name;
	int value;
}
luaterminal_constants[] =
{
	CONST(TK_A),
	CONST(TK_B),
	CONST(TK_C),
	CONST(TK_D),
	CONST(TK_E),
	CONST(TK_F),
	CONST(TK_G),
	CONST(TK_H),
	CONST(TK_I),
	CONST(TK_J),
	CONST(TK_K),
	CONST(TK_L),
	CONST(TK_M),
	CONST(TK_N),
	CONST(TK_O),
	CONST(TK_P),
	CONST(TK_Q),
	CONST(TK_R),
	CONST(TK_S),
	CONST(TK_T),
	CONST(TK_U),
	CONST(TK_V),
	CONST(TK_W),
	CONST(TK_X),
	CONST(TK_Y),
	CONST(TK_Z),
	CONST(TK_0),
	CONST(TK_1),
	CONST(TK_2),
	CONST(TK_3),
	CONST(TK_4),
	CONST(TK_5),
	CONST(TK_6),
	CONST(TK_7),
	CONST(TK_8),
	CONST(TK_9),
	CONST(TK_RETURN),
	CONST(TK_ESCAPE),
	CONST(TK_BACKSPACE),
	CONST(TK_TAB),
	CONST(TK_SPACE),
	CONST(TK_MINUS),
	CONST(TK_EQUALS),
	CONST(TK_LBRACKET),
	CONST(TK_RBRACKET),
	CONST(TK_BACKSLASH),
	CONST(TK_SEMICOLON),
	CONST(TK_APOSTROPHE),
	CONST(TK_GRAVE),
	CONST(TK_COMMA),
	CONST(TK_PERIOD),
	CONST(TK_SLASH),
	CONST(TK_F1),
	CONST(TK_F2),
	CONST(TK_F3),
	CONST(TK_F4),
	CONST(TK_F5),
	CONST(TK_F6),
	CONST(TK_F7),
	CONST(TK_F8),
	CONST(TK_F9),
	CONST(TK_F10),
	CONST(TK_F11),
	CONST(TK_F12),
	CONST(TK_PAUSE),
	CONST(TK_INSERT),
	CONST(TK_HOME),
	CONST(TK_PAGEUP),
	CONST(TK_DELETE),
	CONST(TK_END),
	CONST(TK_PAGEDOWN),
	CONST(TK_RIGHT),
	CONST(TK_LEFT),
	CONST(TK_DOWN),
	CONST(TK_UP),
	CONST(TK_KP_DIVIDE),
	CONST(TK_KP_MULTIPLY),
	CONST(TK_KP_MINUS),
	CONST(TK_KP_PLUS),
	CONST(TK_KP_ENTER),
	CONST(TK_KP_0),
	CONST(TK_KP_1),
	CONST(TK_KP_2),
	CONST(TK_KP_3),
	CONST(TK_KP_4),
	CONST(TK_KP_5),
	CONST(TK_KP_6),
	CONST(TK_KP_7),
	CONST(TK_KP_8),
	CONST(TK_KP_9),
	CONST(TK_KP_PERIOD),
	CONST(TK_SHIFT),
	CONST(TK_CONTROL),
	CONST(TK_MOUSE_LEFT),
	CONST(TK_MOUSE_RIGHT),
	CONST(TK_MOUSE_MIDDLE),
	CONST(TK_MOUSE_X1),
	CONST(TK_MOUSE_X2),
	CONST(TK_MOUSE_MOVE),
	CONST(TK_MOUSE_WHEEL),
	CONST(TK_MOUSE_X),
	CONST(TK_MOUSE_Y),
	CONST(TK_MOUSE_PIXEL_X),
	CONST(TK_MOUSE_PIXEL_Y),
	CONST(TK_MOUSE_WHEEL),
	CONST(TK_MOUSE_CLICKS),
	CONST(TK_WIDTH),
	CONST(TK_HEIGHT),
	CONST(TK_CELL_WIDTH),
	CONST(TK_CELL_HEIGHT),
	CONST(TK_COLOR),
	CONST(TK_BKCOLOR),
	CONST(TK_LAYER),
	CONST(TK_COMPOSITION),
	CONST(TK_CHAR),
	CONST(TK_WCHAR),
	CONST(TK_EVENT),
	CONST(TK_FULLSCREEN),
	CONST(TK_CLOSE),
	CONST(TK_RESIZED),
	CONST(TK_OFF),
	CONST(TK_ON),
	CONST(TK_INPUT_NONE),
	CONST(TK_INPUT_CANCELLED)
};

int luaopen_BearLibTerminal(lua_State* L)
{
	BearLibTerminal::Module liblua = BearLibTerminal::Module::GetProviding("lua_gettop");
	if (!liblua)
	{
		LOG(Error, "Lua provider module was not found");
		return 1; // This will cause lua runtime to fail on table dereferencing, thus notifying user that something went wrong
	}

	bool version_52 = liblua.Probe("lua_tonumberx") != nullptr;

	lua_gettop = (PFNLUAGETTOP)liblua["lua_gettop"];
	lua_createtable = (PFNLUACREATETABLE)liblua["lua_createtable"];
	luaL_checkstack = (PFNLUALCHECKSTACK)liblua["luaL_checkstack"];
	lua_pushvalue = (PFNLUAPUSHVALUE)liblua["lua_pushvalue"];
	lua_pushcclosure = (PFNLUAPUSHCCLOSURE)liblua["lua_pushcclosure"];
	lua_setfield = (PFNLUASETFIELD)liblua["lua_setfield"];
	lua_settop = (PFNLUASETTOP)liblua["lua_settop"];
	lua_pushnumber = (PFNLUAPUSHNUMBER)liblua["lua_pushnumber"];
	lua_pushinteger = (PFNLUAPUSHINTEGER)liblua["lua_pushinteger"];
	lua_tolstring = (PFNLUATOSTRING)liblua["lua_tolstring"];
	lua_rawgeti = (PFNLUARAWGETI)liblua["lua_rawgeti"];
	lua_type = (PFNLUATYPE)liblua["lua_type"];
	lua_pushboolean = (PFNLUAPUSHBOOLEAN)liblua["lua_pushboolean"];
	lua_gettable = (PFNLUAGETTABLE)liblua["lua_gettable"];
	lua_pushstring = (PFNLUAPUSHSTRING)liblua["lua_pushstring"];
	lua_getfield = (PFNLUAGETFIELD)liblua["lua_getfield"];
	lua_insert = (PFNLUAINSERT)liblua["lua_insert"];
	lua_replace = (PFNLUAREPLACE)liblua["lua_replace"];
	lua_error = (PFNLUAERROR)liblua["lua_error"];

	if (version_52)
	{
		// Lua 5.2 has lua_tonumber and lua_tointeger as preprocessor macro

		lua_tonumberx = (PFNLUATONUMBERX)liblua["lua_tonumberx"];
		lua_tonumber = &lua_tonumber_52;

		lua_tointegerx = (PFNLUATOINTEGERX)liblua["lua_tointegerx"];
		lua_tointeger = &lua_tointeger_52;

		lua_pcallk = (PFNLUAPCALLK)liblua["lua_pcallk"];
		lua_pcall = &lua_pcall_52;

		lua_rawlen = (PFNLUARAWLEN)liblua["lua_rawlen"];
		lua_objlen = &lua_objlen_52;
	}
	else
	{
		lua_tonumber = (PFNLUATONUMBER)liblua["lua_tonumber"];
		lua_tointeger = (PFNLUATOINTEGER)liblua["lua_tointeger"];
		lua_pcall = (PFNLUAPCALL)liblua["lua_pcall"];
		lua_objlen = (PFNLUAOBJLEN)liblua["lua_objlen"];
	}

	// Make module table
	luaL_newlib(L, luaterminal_lib);

	// Constants
	for (auto& i: luaterminal_constants)
	{
		lua_pushnumber(L, i.value);
		lua_setfield(L, -2, i.name);
	}

	return 1;
}
