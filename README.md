BearLibTerminal provides a pseudoterminal window with a grid of character cells and a simple yet powerful API for flexible textual output and uncomplicated input processing.

## Features ##

* Unicode support: you can use UTF-8 or UTF-16/32 wide strings easily.
* Support for bitmap and vector (TrueType) fonts and tilesets.
* Extended output facilities: tile composition, alignment, offsets.
* High performance (uses OpenGL).
* Keyboard and mouse support.
* Bindings for several programming languages: С/С++, C#, Lua, Pascal, Python, Ruby.
* Windows and Linux support.


## Using ##

Some notes about using it with different languages or compilers:

### C/C++ ###

Visual C++ projects should be linked against BearLibTerminal.lib import library (specify it in the additional linker dependencies).

With MinGW projects having the .lib file around may break things, so do not copy it along and point compiler to the .dll/.so instead:

    g++ -I/path/to/header -L/path/to/dll main.cpp -lBearLibTerminal -o app.exe

### Lua ###

Wrapper for Lua is built-in. You need to use a regular Lua runtime and place BearLibTerminal binary in a suitable location (e. g. in the same directory as script). For Linux you'll also need to rename the .so to just 'BearLibTerminal.so' (dropping the 'lib' prefix). After that it would be possible to import the library the usual way:

    local T = require "BearLibTerminal"


## Building ##

BearLibTerminal is a language-agnostic dynamic-link library (.dll or .so), therefore you generally do not have to build it yourself and may simply use the prebuilt binaries.

To build BearLibTerminal you will need CMake and a recent GCC/MinGW compiler. For Linux any GCC version 4.6.3 and above will do. For Windows there are several MinGW builds with various quirks, using [TDM-GCC](http://tdm-gcc.tdragon.net/) or [mingw-builds](http://mingw-w64.org/doku.php/download/mingw-builds) (a flavour of mingw-w64) is recommended. MinGW compiler must use Posix thread model.


## License ##

The library is licensed mainly under the MIT license with a few parts under other permissive licenses.