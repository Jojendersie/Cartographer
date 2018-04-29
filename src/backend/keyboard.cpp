#include "ca/gui/backend/keyboard.hpp"
#include <cstring>

namespace ca { namespace gui {

	KeyboardState::KeyboardState()
	{
		memset(keys, 0, 232 * sizeof(ca::gui::KeyboardState::KeyState));
	}

	void KeyboardState::clear()
	{
		for(int i = 0; i < 232; ++i)
		{
			if(keys[i] & KeyState::DOWN)
				keys[i] = KeyState::PRESSED;
			if(keys[i] & KeyState::UP)
				keys[i] = KeyState::RELEASED;
		}

		characterInput = "";
		anyKeyChanged = false;
	}

	bool KeyboardState::isKeyDown(Key _key) const
	{
		return keys[int(_key)] == KeyState::DOWN;
	}

	bool KeyboardState::isKeyPressed(Key _key) const
	{
		return keys[int(_key)] == KeyState::DOWN
			|| keys[int(_key)] == KeyState::PRESSED;
	}

	bool KeyboardState::isControlPressed() const
	{
		return isKeyPressed(Key::CONTROL_LEFT) || isKeyPressed(Key::CONTROL_RIGHT);
	}

	bool KeyboardState::isShiftPressed() const
	{
		return isKeyPressed(Key::SHIFT_LEFT) || isKeyPressed(Key::SHIFT_RIGHT);
	}
	
	bool KeyboardState::isAltPressed() const
	{
		return isKeyPressed(Key::ALT_LEFT) || isKeyPressed(Key::ALT_RIGHT);
	}



	static uint8 GLFW_TO_CAGUI_KEYMAP[349] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		44, 0, 0, 0, 0, 0, 0, 52,
		0, 0, 0, 0, 54, 45, 55, 56,
		30, 31, 32, 33, 34, 35, 36, 37,
		38, 39, 0, 51, 0, 46, 0, 0,
		0, 4, 5, 6, 7, 8, 9, 10,
		11, 12, 13, 14, 15, 16, 17, 18,
		19, 20, 21, 22, 23, 24, 25, 26,
		27, 28, 29, 47, 49, 48, 0, 0,
		53, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 50, 100, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		41, 40, 43, 42, 73, 76, 79, 80,
		81, 82, 75, 78, 74, 77, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		57, 71, 83, 70, 72, 0, 0, 0,
		0, 0, 58, 59, 60, 61, 62, 63,
		64, 65, 66, 67, 68, 69, 104, 105,
		106, 107, 108, 109, 110, 111, 112, 113,
		114, 115, 0, 0, 0, 0, 0, 0, 98,
		89, 90, 91, 92, 93, 94, 95, 96,
		97, 99, 84, 85, 86, 87, 88, 103,
		0, 0, 0, 225, 224, 226, 227, 229,
		228, 230, 231, 101
	};

	KeyboardState::Key KeyboardState::mapGLFWToCaGUIKey(int _glfwKey)
	{
		return static_cast<KeyboardState::Key>(GLFW_TO_CAGUI_KEYMAP[_glfwKey]);
	}

	void KeyboardState::updateFromGLFWInput(int _glfwKey, int _glfwAction)
	{
		if(_glfwAction == 1) { // GLFW_PRESS
			keys[GLFW_TO_CAGUI_KEYMAP[_glfwKey]] = ca::gui::KeyboardState::DOWN;
			anyKeyChanged = true;
		} else if(_glfwAction == 0) { // GLFW_RELEASE
			keys[GLFW_TO_CAGUI_KEYMAP[_glfwKey]] = ca::gui::KeyboardState::UP;
			anyKeyChanged = true;
		} else if(_glfwAction == 2) { // GLFW_REPEAT
			keys[GLFW_TO_CAGUI_KEYMAP[_glfwKey]] = ca::gui::KeyboardState::DOWN;
			anyKeyChanged = true;
		}
	}

}} // namespace ca::gui
