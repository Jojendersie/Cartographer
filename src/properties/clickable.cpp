#pragma once

#include "ca/gui/properties/clickable.hpp"
#include "ca/gui/widgets/widget.hpp"
#include "ca/gui/guimanager.hpp"
#include <ctime>

namespace ca { namespace gui {

	Clickable::Clickable(Widget* _thisWidget) :
		m_clickingEnabled(true)
	{
		for(int b = 0; b < 8; ++b)
		{
			m_lastClick[b] = -10000.0f;
			m_buttonDownReceived[b] = false;
		}
		_thisWidget->registerMouseInputComponent(this);
	}

	Clickable::~Clickable()
	{
	}

	bool Clickable::processInput(Widget & _thisWidget, const MouseState & _mouseState, bool _cursorOnWidget, bool & _ensureNextInput)
	{
		if(!m_clickingEnabled) return false;
		bool usedInput = false;
		if(_cursorOnWidget)
		{
			// Handle each button
			for(int b = 0; b < 8; ++b)
			{
				if(_mouseState.buttons[b] & MouseState::DOWN)
				{
					for(size_t i = 0; i < m_changeFuncs.size(); ++i)
						if(m_statesMasks[i] & MouseState::DOWN)
							m_changeFuncs[i](&_thisWidget, _mouseState.position, b, MouseState::DOWN);
					m_buttonDownReceived[b] = true;
				}

				if(_mouseState.buttons[b] & MouseState::UP)
				{
					for(size_t i = 0; i < m_changeFuncs.size(); ++i)
						if(m_statesMasks[i] & MouseState::UP)
							m_changeFuncs[i](&_thisWidget, _mouseState.position, b, MouseState::UP);
					if(m_buttonDownReceived[b])
					{
						float currentTime = clock() / float(CLOCKS_PER_SEC);
						if(currentTime - m_lastClick[b] <= 0.5f)// TODO: threshold
						{
							m_lastClick[b] = -10000.0f;
							for(size_t i = 0; i < m_changeFuncs.size(); ++i)
								if(m_statesMasks[i] & MouseState::DBL_CLICKED)
									m_changeFuncs[i](&_thisWidget, _mouseState.position, b, MouseState::DBL_CLICKED);
						} else {
							m_lastClick[b] = currentTime;
							for(size_t i = 0; i < m_changeFuncs.size(); ++i)
								if(m_statesMasks[i] & MouseState::CLICKED)
									m_changeFuncs[i](&_thisWidget, _mouseState.position, b, MouseState::CLICKED);
						}
					}
					m_buttonDownReceived[b] = false;
				}

				if(_mouseState.buttons[b] & MouseState::PRESSED)
				{
					for(size_t i = 0; i < m_changeFuncs.size(); ++i)
						if(m_statesMasks[i] & MouseState::PRESSED)
							m_changeFuncs[i](&_thisWidget, _mouseState.position, b, MouseState::PRESSED);
				}
			}
			usedInput = true;
		}

		// If mouse is released somewhere else clear the state
		for(int b = 0; b < 8; ++b)
			if(!_mouseState.buttons[b] || _mouseState.buttons[b] & MouseState::UP)
				m_buttonDownReceived[b] = false;

		usedInput |= isAnyButtonDown();
		// TODO: is this focus handling still necessary?
		if(usedInput)
			GUIManager::setMouseFocus(&_thisWidget, _mouseState.anyButtonDown || _mouseState.anyButtonPressed);
		return usedInput;
	}

	void Clickable::addOnButtonChangeFunc(OnButtonChange _callback, MouseState::ButtonState _stateMask)
	{
		m_changeFuncs.push_back(move(_callback));
		m_statesMasks.push_back(_stateMask);
	}


}} // namespace ca::gui
