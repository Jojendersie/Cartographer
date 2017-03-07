#include "ca/gui/backend/keyboard.hpp"
#include <cstring>

namespace ca { namespace gui {

	KeyboardState::KeyboardState()
	{
		memset(keys, 232, 0);
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
	}

	bool KeyboardState::isKeyDown(Key _key) const
	{
		return keys[int(_key)] == KeyState::DOWN
			|| keys[int(_key)] == KeyState::PRESSED;
	}

}} // namespace ca::gui
