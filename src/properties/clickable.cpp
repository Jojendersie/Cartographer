#pragma once

#include "properties/clickable.hpp"

namespace cag {

	Clickable::Clickable(RefFrame* _selfFrame) :
		m_clickRegion(_selfFrame),
		m_deleteRegion(false)
	{
		for(int b = 0; b < 5; ++b)
		{
			m_lastClick[b] = -10000.0f;
			m_buttonDownReceived[b] = false;
		}
	}

	Clickable::~Clickable()
	{
		if(m_deleteRegion)
			delete m_clickRegion;
	}

	bool Clickable::processInput(const MouseState& _mouseState)
	{
		if(m_clickRegion->isMouseOver(_mouseState.position))
		{
			// Handle each button
			for(int b = 0; b < 5; ++b)
			{
				// For each callback individually
				for(size_t i = 0; i < m_changeFuncs.size(); ++i)
				{
					if(_mouseState.buttons[b] & MouseState::DOWN)
					{
						if(m_statesMasks[i] & MouseState::DOWN)
							m_changeFuncs[i](_mouseState.position, b, MouseState::DOWN);
						m_buttonDownReceived[b] = true;
					}

					if(_mouseState.buttons[b] & MouseState::UP)
					{
						if(m_statesMasks[i] & MouseState::UP)
							m_changeFuncs[i](_mouseState.position, b, MouseState::UP);
						if(m_buttonDownReceived[b])
						{
							float currentTime = 0.0f;// TODO: get time
							if(currentTime - m_lastClick[b] <= 0.5f)// TODO: threshold
							{
								m_lastClick[b] = -10000.0f;
								if(m_statesMasks[i] & MouseState::DBL_CLICKED)
									m_changeFuncs[i](_mouseState.position, b, MouseState::DBL_CLICKED);
							} else {
								m_lastClick[b] = currentTime;
								if(m_statesMasks[i] & MouseState::CLICKED)
									m_changeFuncs[i](_mouseState.position, b, MouseState::CLICKED);
							}
						}
						m_buttonDownReceived[b] = false;
					}

					if(_mouseState.buttons[b] & MouseState::PRESSED && m_statesMasks[i] & MouseState::PRESSED)
						m_changeFuncs[i](_mouseState.position, b, MouseState::PRESSED);
				}
			}
			return true;
		}

		// If mouse is released somewhere else clear the state
		for(int b = 0; b < 5; ++b)
			if(!_mouseState.buttons[b] || _mouseState.buttons[b] & MouseState::UP)
				m_buttonDownReceived[b] = false;

		return false;
	}

	void Clickable::addOnButtonChangeFunc(OnButtonChange _callback, MouseState::ButtonState _stateMask)
	{
		m_changeFuncs.push_back(move(_callback));
		m_statesMasks.push_back(_stateMask);
	}

	void Clickable::setClickRegion(IRegion* _region, bool _delete)
	{
		m_clickRegion = _region;
		m_deleteRegion = _delete;
	}

	EllipseRegion::EllipseRegion(RefFrame* _selfFrame) :
		m_selfFrame(_selfFrame)
	{
	}

	bool EllipseRegion::isMouseOver(const Coord& _mousePos)
	{
		// Convert _mousePos to [-1,1] positions inside the frame
		Coord pos = _mousePos - Coord(m_selfFrame->bottom, m_selfFrame->left);
		pos /= Coord(m_selfFrame->top - m_selfFrame->bottom, m_selfFrame->right - m_selfFrame->left);
		pos = pos * 2.0f - 1.0f;
		// Check if the position is inside a circle
		return lensq(pos) <= 1.0f;
	}

} // namespace cag