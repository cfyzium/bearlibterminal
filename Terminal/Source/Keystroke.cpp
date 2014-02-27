#include "Keystroke.hpp"

namespace BearLibTerminal
{
	constexpr std::uint32_t Keystroke::None;
	constexpr std::uint32_t Keystroke::KeyPress;
	constexpr std::uint32_t Keystroke::KeyRelease;
	constexpr std::uint32_t Keystroke::Keys;
	constexpr std::uint32_t Keystroke::MouseMove;
	constexpr std::uint32_t Keystroke::MouseScroll;
	constexpr std::uint32_t Keystroke::Mouse;
	constexpr std::uint32_t Keystroke::Unicode;
	constexpr std::uint32_t Keystroke::All;

	Keystroke::Keystroke(Type type, std::uint8_t scancode): // keypress/keyrelease events
		type(type),
		scancode(scancode),
		character(0),
		x(0), y(0), z(0)
	{ }

	Keystroke::Keystroke(Type type, std::uint8_t scancode, char16_t character): // character-producing keypress event
		type(type),
		scancode(scancode),
		character(character),
		x(0), y(0), z(0)
	{ }

	Keystroke::Keystroke(Type type, std::uint8_t scancode, int x, int y, int z): // mouse events
		type(type),
		scancode(scancode),
		character(0),
		x(x), y(y), z(z)
	{ }
}
