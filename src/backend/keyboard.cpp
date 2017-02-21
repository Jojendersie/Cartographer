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

}} // namespace ca::gui
