#include "ca/gui/backend/mouse.hpp"

namespace ca { namespace gui {

	void MouseState::clear()
	{
		anyButtonPressed = false;
		// Change the mouse button input states from last frame
		for(int i = 0; i < 8; ++i)
		{
			if(buttons[i] & MouseState::DOWN)
				buttons[i] = MouseState::PRESSED;
			if(buttons[i] & MouseState::UP)
				buttons[i] = MouseState::RELEASED;
			if(buttons[i] == MouseState::PRESSED)
				anyButtonPressed = true;
		}
		lastPosition = position;
		anyButtonDown = false;
		anyButtonUp = false;
		deltaScroll = ei::Vec2(0.0f);
	}

	void MouseState::updateFromGLFWInput(int _glfwButton, int _glfwAction)
	{
		if(_glfwButton < 8)
		{
			if(_glfwAction == 1) { // GLFW_PRESS
				buttons[_glfwButton] = MouseState::DOWN;
				anyButtonDown = true;
			} else if(_glfwAction == 0) { // GLFW_RELEASE
				buttons[_glfwButton] = MouseState::UP;
				anyButtonUp = true;
			}
		}
	}

}} // namespace ca::gui