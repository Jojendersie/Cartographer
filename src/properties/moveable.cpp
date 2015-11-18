#pragma once

#include "properties/refframe.hpp"
#include "properties/anchorable.hpp"
#include "properties/moveable.hpp"
#include "backend/mouse.hpp"

namespace cag {

	Moveable::Moveable(RefFrame* _selfFrame, Anchorable* _anchorable) :
		m_refFrame(_selfFrame),
		m_anchorable(_anchorable),
		m_floating(false)
	{
		// TODO: assert _selfFrame != nullptr
	}

	void Moveable::processInput(const MouseState& _mouseState)
	{
		if(m_floating)
		{
			// If key was released stop floating
			if(_mouseState.buttons[0] == MouseState::PRESSED)
			{
				// Move the object
				m_refFrame->sides[SIDE::LEFT] += _mouseState.deltaPos.x;
				m_refFrame->sides[SIDE::RIGHT] += _mouseState.deltaPos.x;
				m_refFrame->sides[SIDE::BOTTOM] += _mouseState.deltaPos.y;
				m_refFrame->sides[SIDE::TOP] += _mouseState.deltaPos.y;
				// Make sure the anchoring does not reset the object
				if(m_anchorable)
					m_anchorable->resetAnchors();
			} else m_floating = false;
		} else {
			// Is the mouse over the component and is the left button pressed?
			m_floating = _mouseState.buttons[0] == MouseState::DOWN
				&& m_refFrame->isMouseOver(_mouseState.position);
		}
	}

} // namespace cag