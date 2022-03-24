#include "ca/gui/properties/clickable.hpp"
#include "ca/gui/widgets/widget.hpp"
#include "ca/gui/guimanager.hpp"
#include <ctime>

namespace ca { namespace gui {

	Clickable::Clickable(Widget* _thisWidget) :
		m_anyButtonDown(false),
		m_clickingEnabled(true),
		m_hasDoubleClickFunc(false)
	{
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
			m_anyButtonDown = (_mouseState.anyButtonDown || _mouseState.anyButtonPressed)
				&& !_mouseState.anyButtonUp;
			// Handle each button
			for(int b = 0; b < 8; ++b)
			{
				for(size_t i = 0; i < m_changeFuncs.size(); ++i)
				{
					if(_mouseState.btnDown(b) && (m_statesMasks[i] & MouseState::DOWN))
						m_changeFuncs[i](&_thisWidget, _mouseState.position, b, MouseState::DOWN);

					if(_mouseState.btnUp(b) && (m_statesMasks[i] & MouseState::UP))
						m_changeFuncs[i](&_thisWidget, _mouseState.position, b, MouseState::UP);

					if(_mouseState.btnPressed(b) && (m_statesMasks[i] & MouseState::PRESSED))
						m_changeFuncs[i](&_thisWidget, _mouseState.position, b, MouseState::PRESSED);

					if(_mouseState.btnDblClicked(b) && (m_statesMasks[i] & MouseState::DBL_CLICKED))
						m_changeFuncs[i](&_thisWidget, _mouseState.position, b, MouseState::DBL_CLICKED);
					
					// Don't trigger simple and double click at the same time
					if(_mouseState.btnClicked(b) && (m_statesMasks[i] & MouseState::CLICKED)
						&& !(m_hasDoubleClickFunc && _mouseState.btnDblClicked(b)))
						m_changeFuncs[i](&_thisWidget, _mouseState.position, b, MouseState::CLICKED);
				}
			}
			usedInput = true;
		}

		return usedInput;
	}

	void Clickable::addOnButtonChangeFunc(OnButtonChange _callback, MouseState::ButtonState _stateMask)
	{
		m_changeFuncs.push_back(move(_callback));
		m_statesMasks.push_back(_stateMask);
		if(_stateMask & MouseState::DBL_CLICKED)
			m_hasDoubleClickFunc = true;
	}


}} // namespace ca::gui
