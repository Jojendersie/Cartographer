#include "ca/gui/misc/clickhandler.hpp"
#include "ca/gui/backend/mouse.hpp"
#include "ca/gui/widgets/widget.hpp"
#include <ctime>

namespace ca { namespace gui {

	ClickHandler::ClickHandler()
	{
		m_mouseDown = nullptr;
		for(int i = 0; i < 8; ++i) {
			m_buttonDownReceived[i] = false;
			m_lastClick[i] = -10000.0f;
		}
	}

	void ClickHandler::handleUpEvents(struct MouseState& _mouseState)
	{
		// Check for click and dbl-click events.
		if(_mouseState.anyButtonUp)
		{
			if( m_mouseDown && m_mouseDown->isMouseOver(_mouseState.position) )
			{
				float now = clock() / float(CLOCKS_PER_SEC);
				for(int b = 0; b < 8; ++b)
				{
					if( _mouseState.btnUp(b) && (m_buttonDownReceived[b] || _mouseState.btnDown(b)) )
					{
						_mouseState.buttons[b] = MouseState::ButtonState(_mouseState.buttons[b] | MouseState::CLICKED);
						if(now - m_lastClick[b] <= 0.5f)// TODO: threshold parameter
							_mouseState.buttons[b] = MouseState::ButtonState(_mouseState.buttons[b] | MouseState::DBL_CLICKED);
						m_lastClick[b] = now;
					}
				}
			}
		}
	}

	void ClickHandler::handleDownEvents(const struct MouseState& _mouseState, const class Widget* _widget)
	{
		// Always clear down state on up or missed-up no matter where
		for(int b = 0; b < 8; ++b)
			if(_mouseState.btnReleased(b) || (_mouseState.btnUp(b) && !_mouseState.btnDown(b)))
				m_buttonDownReceived[b] = false;

		if(_widget && _mouseState.anyButtonDown)
		{
			m_mouseDown = _widget;
			for(int b = 0; b < 8; ++b)
				if(_mouseState.btnDown(b))
					m_buttonDownReceived[b] = true;
		} else if(_mouseState.anyButtonDown)
			m_mouseDown = nullptr;
	}

}} // namespace ca::gui