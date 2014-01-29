/*
 * Luaopen.cpp
 *
 *  Created on: Jan 26, 2014
 *      Author: Cfyz
 */

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#define PSAPI_VERSION 1
#include <psapi.h>
#elif defined(__linux)
#include <dlfcn.h>
#endif

#define BEARLIBTERMINAL_BUILDING_LIBRARY
#include "BearLibTerminal.h"

#include "Log.hpp"
#include <vector>
#include <type_traits>
#include <stddef.h>

#if defined(_WIN32)
typedef HMODULE ModuleHandle;
typedef FARPROC FunctionHandle;

ModuleHandle GetLuaModule()
{
	std::shared_ptr<std::remove_pointer<HMODULE>::type> psapi(LoadLibraryA("Psapi.dll"), FreeLibrary);
	if (!psapi)
	{
		LOG(Error, "Can't load psapi library");
		return nullptr;
	}

	auto enumProcessModules = (BOOL WINAPI (*)(HANDLE, HMODULE*, DWORD, LPDWORD))GetProcAddress(psapi.get(), "EnumProcessModules");
	if (enumProcessModules == nullptr)
	{
		LOG(Error, "Failed to get address of EnumProcessModulesA function in Psapi.dll");
		return nullptr;
	}

	auto getModuleFileNameExW = (DWORD WINAPI (*)(HANDLE, HMODULE, LPWSTR, DWORD))GetProcAddress(psapi.get(), "GetModuleFileNameExW");
	if (getModuleFileNameExW == nullptr)
	{
		LOG(Error, "Failed to get address of GetModuleFileNameExW in Psapi.dll");
		return nullptr;
	}

	HANDLE process = GetCurrentProcess();
	DWORD bytes_needed = 0;
	enumProcessModules(process, nullptr, 0, &bytes_needed);
	size_t n_modules = bytes_needed / sizeof(HMODULE);
	std::vector<HMODULE> modules{n_modules};

	enumProcessModules(process, modules.data(), modules.size()*sizeof(HMODULE), &bytes_needed);
	for (auto module: modules)
	{
		if (GetProcAddress(module, "lua_gettop") != nullptr)
		{
			wchar_t name[MAX_PATH];
			getModuleFileNameExW(process, module, name, sizeof(name)/sizeof(wchar_t));
			LOG(Info, "Using lua provider module \"" << name << "\"");

			return module;
		}
	}

	return nullptr;
}

FunctionHandle GetLuaFunction(ModuleHandle module, const char* name, bool may_fail=false)
{
	FunctionHandle result = GetProcAddress(module, name);
	if (result == nullptr && !may_fail)
	{
		LOG(Error, "Can't get address of function \"" << name << "\"");
	}
	return result;
}
#elif defined(__linux)
typedef void* ModuleHandle;
typedef void* FunctionHandle;

ModuleHandle GetLuaModule()
{
	return dlopen(0, RTLD_NOW|RTLD_GLOBAL);
}

FunctionHandle GetLuaFunction(ModuleHandle module, const char* name)
{
	return dlsym(module, name);
}
#endif

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

PFNLUAGETTOP lua_gettop;
PFNLUACREATETABLE lua_createtable;
PFNLUALCHECKSTACK luaL_checkstack;
PFNLUAPUSHVALUE lua_pushvalue;
PFNLUAPUSHCCLOSURE lua_pushcclosure;
PFNLUASETFIELD lua_setfield;
PFNLUASETTOP lua_settop;
PFNLUAPUSHNUMBER lua_pushnumber;
PFNLUAPUSHINTEGER lua_pushinteger;
PFNLUATOSTRING lua_tolstring;
PFNLUATONUMBER lua_tonumber;
PFNLUATONUMBERX lua_tonumberx;
PFNLUATOINTEGER lua_tointeger;
PFNLUATOINTEGERX lua_tointegerx;
PFNLUARAWGETI lua_rawgeti;
PFNLUATYPE lua_type;
PFNLUAPUSHBOOLEAN lua_pushboolean;
PFNLUAGETTABLE lua_gettable;
PFNLUAPUSHSTRING lua_pushstring;

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
	lua_pushinteger(L, terminal_open());
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
	lua_pushinteger(L, terminal_set8((const int8_t*)s));
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
			lua_pushinteger(L, i);
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

int luaterminal_has_input(lua_State* L)
{
	lua_pushnumber(L, terminal_has_input());
	return 1;
}

int luaterminal_state(lua_State* L)
{
	lua_pushnumber(L, terminal_state(lua_tonumber(L, 1)));
	return 1;
}

int luaterminal_read(lua_State* L)
{
	int code = terminal_read();
	int released = (code & TK_FLAG_RELEASED) > 0;
	code = code & 0xFF;

	lua_pushnumber(L, code);
	lua_pushboolean(L, released);

	return 2;
}

int luaterminal_read_ext(lua_State* L)
{
	int flags = lua_tonumber(L, 1);
	int code = terminal_read_ext(flags);
	int released = 0;

	if (!(flags & TK_READ_CHAR))
	{
		released = (code & TK_FLAG_RELEASED) > 0;
		code = code & 0xFF;
	}

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

int luaterminal_address_of_image(lua_State* L)
{
	// Just cache the image

	return 0;
}

static const luaL_Reg luaterminal_lib[] =
{
	{"open", luaterminal_open},
	{"close", luaterminal_close},
	{"set", luaterminal_set},
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
	{"has_input", luaterminal_has_input},
	{"state", luaterminal_state},
	{"read", luaterminal_read},
	{"read_ext", luaterminal_read_ext},
	{"read_str", luaterminal_read_str},
	{"color_from_name", luaterminal_color_from_name},
	{"color_from_argb", luaterminal_color_from_argb},
	{"address_of_image", luaterminal_address_of_image},
	{NULL, NULL}
};

struct
{
	const char* name;
	int value;
}
luaterminal_constants[] =
{
	{"LBUTTON",          TK_LBUTTON},
	{"RBUTTON",          TK_RBUTTON},
	{"MBUTTON",          TK_MBUTTON},
	{"CLOSE",            TK_CLOSE},
	{"BACK",             TK_BACK},
	{"BACKSPACE",        TK_BACKSPACE},
	{"TAB",              TK_TAB},
	{"RETURN",           TK_RETURN},
	{"SHIFT",            TK_SHIFT},
	{"CONTROL",          TK_CONTROL},
	{"PAUSE",            TK_PAUSE},
	{"ESCAPE",           TK_ESCAPE},
	{"SPACE",            TK_SPACE},
	{"PRIOR",            TK_PRIOR},
	{"NEXT",             TK_NEXT},
	{"END",              TK_END},
	{"HOME",             TK_HOME},
	{"LEFT",             TK_LEFT},
	{"UP",               TK_UP},
	{"RIGHT",            TK_RIGHT},
	{"DOWN",             TK_DOWN},
	{"INSERT",           TK_INSERT},
	{"DELETE",           TK_DELETE},
	{"D0",               TK_0},
	{"D1",               TK_1},
	{"D2",               TK_2},
	{"D3",               TK_3},
	{"D4",               TK_4},
	{"D5",               TK_5},
	{"D6",               TK_6},
	{"D7",               TK_7},
	{"D8",               TK_8},
	{"D9",               TK_9},
	{"A",                TK_A},
	{"B",                TK_B},
	{"C",                TK_C},
	{"D",                TK_D},
	{"E",                TK_E},
	{"F",                TK_F},
	{"G",                TK_G},
	{"H",                TK_H},
	{"I",                TK_I},
	{"J",                TK_J},
	{"K",                TK_K},
	{"L",                TK_L},
	{"M",                TK_M},
	{"N",                TK_N},
	{"O",                TK_O},
	{"P",                TK_P},
	{"Q",                TK_Q},
	{"R",                TK_R},
	{"S",                TK_S},
	{"T",                TK_T},
	{"U",                TK_U},
	{"V",                TK_V},
	{"W",                TK_W},
	{"X",                TK_X},
	{"Y",                TK_Y},
	{"Z",                TK_Z},
	{"GRAVE",            TK_GRAVE},
	{"MINUS",            TK_MINUS},
	{"EQUALS",           TK_EQUALS},
	{"BACKSLASH",        TK_BACKSLASH},
	{"LBRACKET",         TK_LBRACKET},
	{"RBRACKET",         TK_RBRACKET},
	{"SEMICOLON",        TK_SEMICOLON},
	{"APOSTROPHE",       TK_APOSTROPHE},
	{"COMMA",            TK_COMMA},
	{"PERIOD",           TK_PERIOD},
	{"SLASH",            TK_SLASH},
	{"NUMPAD0",          TK_NUMPAD0},
	{"NUMPAD1",          TK_NUMPAD1},
	{"NUMPAD2",          TK_NUMPAD2},
	{"NUMPAD3",          TK_NUMPAD3},
	{"NUMPAD4",          TK_NUMPAD4},
	{"NUMPAD5",          TK_NUMPAD5},
	{"NUMPAD6",          TK_NUMPAD6},
	{"NUMPAD7",          TK_NUMPAD7},
	{"NUMPAD8",          TK_NUMPAD8},
	{"NUMPAD9",          TK_NUMPAD9},
	{"MULTIPLY",         TK_MULTIPLY},
	{"ADD",              TK_ADD},
	{"SUBSTRACT",        TK_SUBTRACT},
	{"DECIMAL",          TK_DECIMAL},
	{"DIVIDE",           TK_DIVIDE},
	{"F1",               TK_F1},
	{"F2",               TK_F2},
	{"F3",               TK_F3},
	{"F4",               TK_F4},
	{"F5",               TK_F5},
	{"F6",               TK_F6},
	{"F7",               TK_F7},
	{"F8",               TK_F8},
	{"F9",               TK_F9},
	{"F10",              TK_F10},
	{"F11",              TK_F11},
	{"F12",              TK_F12},
	{"MOUSE_MOVE",       TK_MOUSE_MOVE},
	{"MOUSE_SCROLL",     TK_MOUSE_SCROLL},
	{"WINDOW_RESIZE",    TK_WINDOW_RESIZE},
	{"MOUSE_X",          TK_MOUSE_X},
	{"MOUSE_Y",          TK_MOUSE_Y},
	{"MOUSE_PIXEL_X",    TK_MOUSE_PIXEL_X},
	{"MOUSE_PIXEL_Y",    TK_MOUSE_PIXEL_Y},
	{"MOUSE_WHEEL",      TK_MOUSE_WHEEL},
	{"CELL_WIDTH",       TK_CELL_WIDTH},
	{"CELL_HEIGHT",      TK_CELL_HEIGHT},
	{"WIDTH",            TK_WIDTH},
	{"HEIGHT",           TK_HEIGHT},
	{"COMPOSITION",      TK_COMPOSITION},
	{"COLOR",            TK_COLOR},
	{"BKCOLOR",          TK_BKCOLOR},
	{"LAYER",            TK_LAYER},
	{"COMPOSITION_OFF",  TK_COMPOSITION_OFF},
	{"COMPOSITION_ON",   TK_COMPOSITION_ON},
	{"INPUT_NONE",       TK_INPUT_NONE},
	{"INPUT_CANCELLED",  TK_INPUT_CANCELLED},
	{"INPUT_CALL_AGAIN", TK_INPUT_CALL_AGAIN},
	{"READ_CHAR",        TK_READ_CHAR},
	{"READ_NOREMOVE",    TK_READ_NOREMOVE}
};

int luaopen_BearLibTerminal(lua_State* L)
{
	auto module = GetLuaModule();
	if (module == nullptr)
	{
		LOG(Error, "Lua provider module was not found");
		return 1; // This will cause lua runtime to fail on table dereferencing, thus notifying user that something went wrong
	}

	bool version_52 = GetLuaFunction(module, "lua_tonumberx", true) != nullptr;

	lua_gettop = (PFNLUAGETTOP)GetLuaFunction(module, "lua_gettop");
	lua_createtable = (PFNLUACREATETABLE)GetLuaFunction(module, "lua_createtable");
	luaL_checkstack = (PFNLUALCHECKSTACK)GetLuaFunction(module, "luaL_checkstack");
	lua_pushvalue = (PFNLUAPUSHVALUE)GetLuaFunction(module, "lua_pushvalue");
	lua_pushcclosure = (PFNLUAPUSHCCLOSURE)GetLuaFunction(module, "lua_pushcclosure");
	lua_setfield = (PFNLUASETFIELD)GetLuaFunction(module, "lua_setfield");
	lua_settop = (PFNLUASETTOP)GetLuaFunction(module, "lua_settop");
	lua_pushnumber = (PFNLUAPUSHNUMBER)GetLuaFunction(module, "lua_pushnumber");
	lua_pushinteger = (PFNLUAPUSHINTEGER)GetLuaFunction(module, "lua_pushinteger");
	lua_tolstring = (PFNLUATOSTRING)GetLuaFunction(module, "lua_tolstring");
	lua_rawgeti = (PFNLUARAWGETI)GetLuaFunction(module, "lua_rawgeti");
	lua_type = (PFNLUATYPE)GetLuaFunction(module, "lua_type");
	lua_pushboolean = (PFNLUAPUSHBOOLEAN)GetLuaFunction(module, "lua_pushboolean");
	lua_gettable = (PFNLUAGETTABLE)GetLuaFunction(module, "lua_gettable");
	lua_pushstring = (PFNLUAPUSHSTRING)GetLuaFunction(module, "lua_pushstring");

	if (version_52)
	{
		// Lua 5.2 has lua_tonumber and lua_tointeger as preprocessor macro

		lua_tonumberx = (PFNLUATONUMBERX)GetLuaFunction(module, "lua_tonumberx");
		lua_tonumber = &lua_tonumber_52;

		lua_tointegerx = (PFNLUATOINTEGERX)GetLuaFunction(module, "lua_tointegerx");
		lua_tointeger = &lua_tointeger_52;
	}
	else
	{
		lua_tonumber = (PFNLUATONUMBER)GetLuaFunction(module, "lua_tonumber", true);
		lua_tointeger = (PFNLUATOINTEGER)GetLuaFunction(module, "lua_tointeger", true);
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
