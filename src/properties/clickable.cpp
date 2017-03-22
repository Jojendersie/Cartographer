#pragma once

#include "ca/gui/properties/clickable.hpp"
#include "ca/gui/widgets/widget.hpp"
#include <ctime>

namespace ca { namespace gui {

	Clickable::Clickable(Widget* _thisWidget) :
		m_widget(_thisWidget)
	{
		for(int b = 0; b < 8; ++b)
		{
			m_lastClick[b] = -10000.0f;
			m_buttonDownReceived[b] = false;
		}
	}

	Clickable::~Clickable()
	{
	}

	bool Clickable::processInput(const MouseState& _mouseState)
	{
		if(m_widget->getRegion()->isMouseOver(_mouseState.position))
		{
			// Handle each button
			for(int b = 0; b < 8; ++b)
			{
				if(_mouseState.buttons[b] & MouseState::DOWN)
				{
					for(size_t i = 0; i < m_changeFuncs.size(); ++i)
						if(m_statesMasks[i] & MouseState::DOWN)
							m_changeFuncs[i](m_widget, _mouseState.position, b, MouseState::DOWN);
					m_buttonDownReceived[b] = true;
				}

				if(_mouseState.buttons[b] & MouseState::UP)
				{
					for(size_t i = 0; i < m_changeFuncs.size(); ++i)
						if(m_statesMasks[i] & MouseState::UP)
							m_changeFuncs[i](m_widget, _mouseState.position, b, MouseState::UP);
					if(m_buttonDownReceived[b])
					{
						float currentTime = clock() / float(CLOCKS_PER_SEC);
						if(currentTime - m_lastClick[b] <= 0.5f)// TODO: threshold
						{
							m_lastClick[b] = -10000.0f;
							for(size_t i = 0; i < m_changeFuncs.size(); ++i)
								if(m_statesMasks[i] & MouseState::DBL_CLICKED)
									m_changeFuncs[i](m_widget, _mouseState.position, b, MouseState::DBL_CLICKED);
						} else {
							m_lastClick[b] = currentTime;
							for(size_t i = 0; i < m_changeFuncs.size(); ++i)
								if(m_statesMasks[i] & MouseState::CLICKED)
									m_changeFuncs[i](m_widget, _mouseState.position, b, MouseState::CLICKED);
						}
					}
					m_buttonDownReceived[b] = false;
				}

				if(_mouseState.buttons[b] & MouseState::PRESSED)
				{
					for(size_t i = 0; i < m_changeFuncs.size(); ++i)
						if(m_statesMasks[i] & MouseState::PRESSED)
							m_changeFuncs[i](m_widget, _mouseState.position, b, MouseState::PRESSED);
				}
			}
			return true;
		}

		// If mouse is released somewhere else clear the state
		for(int b = 0; b < 8; ++b)
			if(!_mouseState.buttons[b] || _mouseState.buttons[b] & MouseState::UP)
				m_buttonDownReceived[b] = false;

		return isAnyButtonDown();
	}

	void Clickable::addOnButtonChangeFunc(OnButtonChange _callback, MouseState::ButtonState _stateMask)
	{
		m_changeFuncs.push_back(move(_callback));
		m_statesMasks.push_back(_stateMask);
	}


}} // namespace ca::gui
