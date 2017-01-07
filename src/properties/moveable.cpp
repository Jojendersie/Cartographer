#pragma once

#include "ca/gui/properties/refframe.hpp"
#include "ca/gui/properties/anchorable.hpp"
#include "ca/gui/properties/moveable.hpp"
#include "ca/gui/backend/mouse.hpp"

namespace ca { namespace gui {

	Moveable::Moveable(RefFrame* _selfFrame, Anchorable* _anchorable) :
		m_refFrame(_selfFrame),
		m_anchorable(_anchorable),
		m_moving(false)
	{
		// TODO: assert _selfFrame != nullptr
	}

	bool Moveable::processInput(const MouseState& _mouseState)
	{
		if(m_moving)
		{
			// If key was released stop floating
			if(_mouseState.buttons[0] == MouseState::PRESSED)
			{
				// Move the object
				Coord2 deltaPos = _mouseState.deltaPos();
				m_refFrame->sides[SIDE::LEFT] += deltaPos.x;
				m_refFrame->sides[SIDE::RIGHT] += deltaPos.x;
				m_refFrame->sides[SIDE::BOTTOM] += deltaPos.y;
				m_refFrame->sides[SIDE::TOP] += deltaPos.y;
				// Make sure the anchoring does not reset the object
				if(m_anchorable)
					m_anchorable->resetAnchors();
			} else m_moving = false;
		} else {
			// Is the mouse over the component and is the left button pressed?
			m_moving = _mouseState.buttons[0] == MouseState::DOWN
				&& m_refFrame->isMouseOver(_mouseState.position);
		}
		return m_moving;
	}

}} // namespace ca::gui
