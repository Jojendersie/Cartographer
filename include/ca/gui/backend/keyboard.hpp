#pragma once

#include <string>
#include <ei/elementarytypes.hpp>

namespace ca { namespace gui {

	struct KeyboardState
	{
		KeyboardState();

		/// A string with all character strokes since last state actualization.
		/// The string should represent the input order.
		/// The characters should respect the current keyboard layout and modifier keys.
		std::string characterInput;

		/// Flags of hardware key states.
		enum KeyState : uint8
		{
			RELEASED = 0,
			DOWN = 1,
			PRESSED = 2,
			UP = 4,
		};

		/// Physical key names.
		/// USB HID Usage Tables v1.12 (p. 53-60).
		enum class Key
		{
			UNKNOWN = 0,
			A = 4,
			B = 5,
			C = 6,
			D = 7,
			E = 8,
			F = 9,
			G = 10,
			H = 11,
			I = 12,
			J = 13,
			K = 14,
			L = 15,
			M = 16,
			N = 17,
			O = 18,
			P = 19,
			Q = 20,
			R = 21,
			S = 22,
			T = 23,
			U = 24,
			V = 25,
			W = 26,
			X = 27,
			Y = 28,
			Z = 29,
			ONE = 30,
			TWO = 31,
			THREE = 32,
			FOUR = 33,
			FIVE = 34,
			SIX = 35,
			SEVEN = 36,
			EIGHT = 37,
			NINE = 38,
			ZERO = 39,
			ENTER = 40,
			ESCAPE = 41,
			BACKSPACE = 42,
			TAB = 43,
			SPACE = 44,
			MINUS = 45,
			EQUAL = 46,
			LEFT_BRACKET = 47,	// [
			RIGHT_BRACKET = 48,	// ]
			BACKSLASH = 49,
			HASH = 50,
			SEMICOLON = 51,
			APOSTROPHE = 52,
			GRAVE_ACCENT = 53,
			COMMA = 54,
			PERIOD = 55,
			SLASH = 56,
			CAPS_LOCK = 57,
			F1 = 58,
			F2 = 59,
			F3 = 60,
			F4 = 61,
			F5 = 62,
			F6 = 63,
			F7 = 64,
			F8 = 65,
			F9 = 66,
			F10 = 67,
			F11 = 68,
			F12 = 69,
			PRINT_SCREEN = 70,
			SCROLL_LOCK = 71,
			PAUSE = 72,
			INSERT = 73,
			HOME = 74,
			PAGE_UP = 75,
			DELETE = 76,
			END = 77,
			PAGE_DOWN = 78,
			ARROW_RIGHT = 79,
			ARROW_LEFT = 80,
			ARROW_DOWN = 81,
			ARROW_UP = 82,
			NUM_LOCK = 83,
			NUMPAD_DIVIDE = 84,
			NUMPAD_MULTIPLY = 85,
			NUMPAD_SUBTRACT = 86,
			NUMPAD_ADD = 87,
			NUMPAD_ENTER = 88,
			NUMPAD_1 = 89,
			NUMPAD_2 = 90,
			NUMPAD_3 = 91,
			NUMPAD_4 = 92,
			NUMPAD_5 = 93,
			NUMPAD_6 = 94,
			NUMPAD_7 = 95,
			NUMPAD_8 = 96,
			NUMPAD_9 = 97,
			NUMPAD_0 = 98,
			NUMPAD_DELETE = 99,
			BAR = 100,
			MENU = 101,
			NUMPAD_EQUAL = 103,

			F13 = 104,
			F14 = 105,
			F15 = 106,
			F16 = 107,
			F17 = 108,
			F18 = 109,
			F19 = 110,
			F20 = 111,
			F21 = 112,
			F22 = 113,
			F23 = 114,
			F24 = 115,

			// A lot of unusual keys ... fill in if required

			CONTROL_LEFT = 224,
			SHIFT_LEFT = 225,
			ALT_LEFT = 226,
			GUI_LEFT = 227,
			CONTROL_RIGHT = 228,
			SHIFT_RIGHT = 229,
			ALT_RIGHT = 230,
			GUI_RIGHT = 231
		};

		/// State of all physical keys.
		KeyState keys[232];

		/// Reset/change states
		void clear();

		/// Returns true if the of the key is DOWN or PRESSED
		bool isKeyDown(Key _key) const;
	};

}} // namespace ca::gui
