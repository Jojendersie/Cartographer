#include "ca/gui/popupmanager.hpp"
#include "ca/gui/guimanager.hpp"
#include <ca/pa/log.hpp>
#include <ei/2dintersection.hpp>

namespace ca { namespace gui {
	using namespace pa;
	using namespace ei;

	PopupManager::PopupManager() :
		m_popupTime { 0.33f }
	{
	}


	void PopupManager::setPopupTime(float _showAfter)
	{
		if(_showAfter <= 0.0f) {
			pa::logWarning("Popup time must be a positive number, but is ", _showAfter);
			return;
		}
		m_popupTime = _showAfter;
	}


	void PopupManager::showPopup(
		WidgetPtr & _popup,
		const Widget * _originator,
		PopupClosingCondition _closeOn,
		bool _receiveInputs,
		float _closeThreshold)
	{
		if(!_originator->isVisible())
			return; // How can this happen?

		// Make sure the element is not yet on the stack.
		for(auto& it : m_popupStack)
			if(it.widget == _popup) return;

		// Push a new entry
		_popup->showAsPopup(_originator);
		m_popupStack.push_back(PopupInfo{
			_popup,
			_closeOn,
			_closeThreshold,
			_receiveInputs
		});
	}


	void PopupManager::showPopupAt(
		WidgetPtr & _popup,
		const Widget * _originator,
		Coord2 _position,
		PopupClosingCondition _closeOn,
		bool _receiveInputs,
		float _closeThreshold)
	{
		// Try to place the popup at the bottom right of the given position.
		_position.y -= _popup->size().y;
		// If there is not enough place to the right side move to the left.
		float maxW = float(GUIManager::getWidth());
		if(_popup->parent())
			maxW = _popup->parent()->right();
		_position.x += min(0.0f, maxW - (_position.x + _popup->size().x));
		// If it does not fit verticaly toggle it upward.
		float minH = 0.0f;
		if(_popup->parent())
			minH = _popup->parent()->bottom();
		if(_position.y - _popup->size().y < minH)
			_position.y += _popup->size().y + 14.0f;

		_popup->setPosition(_position);
		showPopup(_popup, _originator, _closeOn, _receiveInputs, _closeThreshold);
	}


	void PopupManager::closePopup(Widget * _popup)
	{
		for(auto it = m_popupStack.begin(); it != m_popupStack.end(); ++it)
		{
			if(it->widget == _popup)
			{
				m_popupStack.erase(it);
				break;
			}
		}
		_popup->hide();
	}


	bool PopupManager::processInput(const struct MouseState& _mouseState)
	{
		// Check for exit conditions until something blocks or stack is empty
		bool stackChanged = true;
		while(!m_popupStack.empty() && stackChanged)
		{
			stackChanged = false;
			const float dist = max(0.0f, distance(_mouseState.position, m_popupStack.back().widget->rectangle()));
			if(m_popupStack.back().cond == PopupClosingCondition::MOUSE_DISTANCE)
			{
				if(dist > m_popupStack.back().condThreshold)
				{
					stackChanged = true;
					m_popupStack.back().widget->hide();
					m_popupStack.pop_back();
					continue;
				}
			}

			// Both conditions collapse the popup on outside click.
			if(dist > 0.0f) // Outside
			{
				if(_mouseState.anyButtonDown)
				{
					stackChanged = true;
					m_popupStack.back().widget->hide();
					m_popupStack.pop_back();
					continue;
				}
			}
		}

		// Forward inputs to popups if they need them
		for(auto& it : m_popupStack) // TODO: check order: must be top to bottom!
		{
			const bool mouseOver = it.widget->isMouseOver(_mouseState.position);
			if(mouseOver)
				GUIManager::setMouseOver(it.widget.get());
			//	TODO: remove all the other m_lastClick.... things (Clickable, DropDown, ...)
			if(it.receiveInput)
			{
				if(it.widget->processInput( _mouseState ))
					return true;
			} else {
				// If the topmost element is below the cursor, nothing behind it may receive
				// the input.
				if(mouseOver)
					return false;
			}
		}

		return false;
	}


	void PopupManager::draw() const
	{
		for(auto& it : m_popupStack)
			it.widget->draw();
	}


	void PopupManager::clear()
	{
		m_popupStack.clear();
	}

}} // namespace ca::gui