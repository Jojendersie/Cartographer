#include "ca/gui/properties/resizeable.hpp"
#include "ca/gui/properties/refframe.hpp"
#include "ca/gui/properties/anchorable.hpp"
#include "ca/gui/backend/mouse.hpp"
#include "ca/gui/guimanager.hpp"

namespace ca { namespace gui {

	Resizeable::Resizeable(Widget* _thisWidget) :
		m_resizingEnabled(SIDE_FLAGS::ALL),
		m_active(false),
		m_resizing{false, false, false, false}
	{
		_thisWidget->registerMouseInputComponent(this);
	}

	static void chooseCursor(bool _flags[4])
	{
		if(_flags[SIDE::LEFT] && _flags[SIDE::BOTTOM])		 GUIManager::setCursorType(CursorType::RESIZE_DUP);
		else if(_flags[SIDE::LEFT] && _flags[SIDE::TOP])	 GUIManager::setCursorType(CursorType::RESIZE_DDOWN);
		else if(_flags[SIDE::RIGHT] && _flags[SIDE::TOP])	 GUIManager::setCursorType(CursorType::RESIZE_DUP);
		else if(_flags[SIDE::RIGHT] && _flags[SIDE::BOTTOM]) GUIManager::setCursorType(CursorType::RESIZE_DDOWN);
		else if(_flags[SIDE::LEFT] || _flags[SIDE::RIGHT])	 GUIManager::setCursorType(CursorType::RESIZE_H);
		else if(_flags[SIDE::BOTTOM] || _flags[SIDE::TOP])	 GUIManager::setCursorType(CursorType::RESIZE_V);
	}

	bool Resizeable::processInput(Widget & _thisWidget, const MouseState & _mouseState, bool _cursorOnWidget, bool & _ensureNextInput)
	{
		if(!m_resizingEnabled) return false;
		if(m_active)
		{
			chooseCursor(m_resizing);
			// If key was released stop floating
			if(_mouseState.buttons[0] == MouseState::PRESSED)
			{
				Coord2 deltaPos = _mouseState.deltaPos();
				if(deltaPos != Coord2(0.0f))
				{
					_thisWidget.resize(m_resizing[SIDE::LEFT] ? deltaPos.x : 0.0f,
						m_resizing[SIDE::RIGHT] ? deltaPos.x : 0.0f,
						m_resizing[SIDE::BOTTOM] ? deltaPos.y : 0.0f,
						m_resizing[SIDE::TOP] ? deltaPos.y : 0.0f);
				}
				GUIManager::setMouseFocus(&_thisWidget, true);
				_ensureNextInput = true;
			} else {
				m_active = false;
				if(m_onResizeEnded)
					m_onResizeEnded(&_thisWidget);
			}
			return true;
		} else {
			// Is the cursor within range?
			if(_mouseState.position.x >= _thisWidget.left()-2.0f
				&& _mouseState.position.x <= _thisWidget.right()+2.0f
				&& _mouseState.position.y >= _thisWidget.bottom()-2.0f
				&& _mouseState.position.y <= _thisWidget.top()+2.0f)
			{
				// Is the mouse cursor within a margin around the reference frame?
				m_resizing[SIDE::LEFT] = ((m_resizingEnabled & SIDE_FLAGS::LEFT) != 0)
					&& (abs(_thisWidget.left() - _mouseState.position.x) <= 2.0f);
				m_resizing[SIDE::RIGHT] = ((m_resizingEnabled & SIDE_FLAGS::RIGHT) != 0)
					&& (abs(_thisWidget.right() - _mouseState.position.x) <= 2.0f);
				m_resizing[SIDE::BOTTOM] = ((m_resizingEnabled & SIDE_FLAGS::BOTTOM) != 0)
					&& (abs(_thisWidget.bottom() - _mouseState.position.y) <= 2.0f);
				m_resizing[SIDE::TOP] = ((m_resizingEnabled & SIDE_FLAGS::TOP) != 0)
					&& (abs(_thisWidget.top() - _mouseState.position.y) <= 2.0f);
				bool anyFlag = (m_resizing[0] || m_resizing[1] || m_resizing[2] || m_resizing[3]);

				// Set a cursor if in margin
				chooseCursor(m_resizing);

				m_active = _mouseState.buttons[0] == MouseState::DOWN && anyFlag;
				if(m_active)
				{
					GUIManager::setMouseFocus(&_thisWidget, true);
					_ensureNextInput = true;
				}
				return anyFlag;
			}
		}
		return false;
	}

}} // namespace ca::gui
