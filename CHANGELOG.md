### 0.15.2 (Unreleased)

- Retrieve clipboard contents via `terminal_get("clipboard")`.
- Fix bitmap tileset reverse codepage (sparse tileset) handling.
- `input.cursor-blink-rate=0` disables cursor blinking in `terminal_read_str()`. 

### 0.15.1 (2017-01-18)

- Change default hinting to font's native hinter.
- Add `hinting` TrueType font attribute.
- Make Python wrapper look for the library binary near the executable (should help packaging the application). 
- Minor fix in the sample application.

### 0.15.0 (2017-01-09)

- Add bbox and alignment parameters to `print()` function (instead of using in-string formatting tags).
- Add bbox parameters to `measure()` function.
- Add `dead-center` tile alignment (centering by image's center of mass).
- Add `use-box-drawing` and `use-block-elements` truetype font attributes (disables auto-generation of respective characters).
- Add text resource descriptors (specifying resources inside a configuration string, e. g. codepages).
- Add version property (available via `get()`).
- Fix TrueType font alignment at arbitrary tile size.
- Fix minor memory leaks (in X11Window and TrueType font constructors).
- Fix keybad keys not producing character codes (`TK_CHAR`/`TK_WCHAR` states).

### 0.14.12 (2016-12-01)

- Fix mapping of first 32 characters of CP437 codepage.
- Fix priority of auto-generated Box Drawing tiles.
- Fix crashing Python interpeter when exiting suddenly.
- Add support for grayscale bitmap font images w/o transparency.
- Setting a font without specifying a size parameter is not allowed anymore.
- C#: minor wrapper improvements (function overloads and Size type support in Set/Get).

### 0.14.11 (2016-10-30)

- Fix 'resize' parameter in bitmap tilesets (also change its meaning to target tile size).
- Fix toggling fullscreen in Linux (_DIALOG window type is incompatible with _FULLSCREEN).
- Fix crash under Wine (dynamic library was being unloaded way too early).
- Fix crash on the older hardware (sprite textures were not honoring the lack of NPOTD support).
- Fix out-of-bounds reads in bilinear filter (produced artifacts on bitmap borders).
- Add missing TK\_MOUSE\_SCROLL constant in Lua binding.

### 0.14.10 (2016-10-16)

- The window is now centered on a screen at startup (see [issue #15](https://bitbucket.org/cfyzium/bearlibterminal/issues/15/os-x-window-isnt-centered-in-the-screen)).
- Add Cmd+Q app menu for OS X ([issue #16](https://bitbucket.org/cfyzium/bearlibterminal/issues/16/os-x-command-q-doesnt-work)).
- Fix first redraw on OS X 10.11 ([issue #14](https://bitbucket.org/cfyzium/bearlibterminal/issues/14/loading-takes-a-long-time-and-cant-be)).
- Fix flooding mouse-movement events (again, finally).
- Python wrapper fixes (performance optimizations, check() behaviour, a bug in get(); see [issue #17](https://bitbucket.org/cfyzium/bearlibterminal/issues/17/python-optimizations), [issue #8](https://bitbucket.org/cfyzium/bearlibterminal/issues/8/check-bug-in-the-python-wrapper)).
- Fix dynamically generated character U+2523.

### 0.14.8 (2016-09-06)

- Setting custom colors in the palette (available later via `color_from_name()` and the `[color=name]` formatting tag)
- Specifying custom colors in the 'Palette' section of a configuration file.
- Fix filtered events ordering (some events could have been lost before).
- Pascal: Delphi-compatible Pascal wrapper.

### 0.14.7 (2016-08-31)

- Fixed parsing resource names (some absolute paths were incorrectly treated as memory addresses).

### 0.14.6 (2016-08-30)

- 'System' events like TK_CLOSE are always read regardless of input filter (no need to explicitly specify them while setting the filter).

### 0.14.5 (2016-08-29)

- Add support for Lua 5.3
- Fix reading some configuration options.
- Fix blocking on input when the window is not yet shown (would deadlock).
- Default input filter is 'keyboard, system' now (terminal read is equivalent to 'press any key').

### 0.14.4 (2016-08-17)

- Fix reading/updating single parameters in configuration file.

### 0.14.3 (2016-07-22)

- Fix setting font via configuration file. Yes, again =(

### 0.14.2 (2016-07-21)

- Fix library deinitialization routine (cleaning texture atlas).

### 0.14.0 (2016-04-22)

- Add Alt key state and event (TK_ALT).
- Add named alternative fonts.
- Uniform in-memory resource loading.

### 0.13.2 (2016-03-28)

- Add IPython REPL integration.
- Fix bug in WinAPI window event polling.

### 0.13.1 (2016-03-28)

- Fix (revert) accidental default input filter changes.

### 0.13.0 (2016-03-26)

- Add OS X support.
