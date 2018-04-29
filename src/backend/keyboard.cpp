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

}} // namespace ca::gui
