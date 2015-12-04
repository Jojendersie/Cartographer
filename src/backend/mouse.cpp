#include "backend/mouse.hpp"

namespace ca { namespace gui {

	void MouseState::clear()
	{
		// Change the mouse button input states from last frame
		for(int i = 0; i < 8; ++i)
		{
			if(buttons[i] & MouseState::DOWN)
				buttons[i] = MouseState::PRESSED;
			if(buttons[i] & MouseState::UP)
				buttons[i] = MouseState::RELEASED;
		}
		lastPosition = position;
		anyButtonDown = false;
		anyButtonUp = false;
	}

}} // namespace ca::gui