#include "ca/gui/properties/refframe.hpp"
#include "ca/gui/properties/anchorable.hpp"
#include "ca/gui/properties/moveable.hpp"
#include "ca/gui/backend/mouse.hpp"
#include "ca/gui/widgets/widget.hpp"
#include "ca/gui/guimanager.hpp"

namespace ca { namespace gui {

	Moveable::Moveable(Widget* _thisWidget) :
		m_movingEnabled(true),
		m_moving(false),
		m_useRestriction(false)
	{
		// TODO: assert _selfFrame != nullptr
		_thisWidget->registerMouseInputComponent(this);
	}

	bool Moveable::processInput(Widget& _thisWidget, const MouseState& _mouseState, bool _cursorOnWidget, bool& _ensureNextInput)
	{
		if(!m_movingEnabled) return false;
		if(m_moving)
		{
			// If key was released stop floating
			if(_mouseState.buttons[0] == MouseState::PRESSED)
			{
				// Move the float position and get a new real position
				m_floatingPosition += _mouseState.deltaPos();
				Coord2 targetPosition = m_floatingPosition;
				if(m_useRestriction)
					targetPosition = m_snapFunction(m_floatingPosition);
				// Move the object
				Coord2 deltaPos = targetPosition - _thisWidget.position();
				_thisWidget.move(deltaPos);
			} else m_moving = false;
		} else {
			// Is the mouse over the component and is the left button pressed?
			m_moving = _mouseState.buttons[0] == MouseState::DOWN
				&& _cursorOnWidget;
			// Get the current position as starting float-position
			if(m_moving)
				m_floatingPosition = _thisWidget.position();
		}
		if(m_moving)
		{
			GUIManager::setCursorType(CursorType::MOVE);
			GUIManager::setMouseFocus(&_thisWidget, true);
			_ensureNextInput = true;
		}
		return m_moving;
	}

	void Moveable::restrictMovement(RestrictionFunction _restrictionFunction)
	{
		m_snapFunction = move(_restrictionFunction);
		m_useRestriction = true;
	}

}} // namespace ca::gui
