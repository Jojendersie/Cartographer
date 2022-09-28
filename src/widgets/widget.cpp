#include "ca/gui/widgets/widget.hpp"
#include "ca/gui/guimanager.hpp"
#include <ca/pa/log.hpp>

namespace ca { namespace gui {

	using namespace pa;

	Widget::Widget() :
		m_enabled(true),
		m_keyboardFocusable(false),
		m_visible(true),
		m_region(nullptr),
		m_activeComponent(nullptr),
		m_parent(nullptr)
	{
	}

	Widget::~Widget()
	{
		GUIManager::defocus(this);
	}

	void Widget::setSize(const Coord2& _size)
	{
		setFrame(
			left(),
			bottom(),
			left() + _size.x,
			bottom() + _size.y
		);
	}


	void Widget::setPosition(const Coord2& _position)
	{
		setFrame(
			_position.x,
			_position.y,
			_position.x + width(),
			_position.y + height()
		);
	}


	void Widget::move(const Coord2 & _deltaPosition)
	{
		setFrame(
			left() + _deltaPosition.x,
			bottom() + _deltaPosition.y,
			right() + _deltaPosition.x,
			top() + _deltaPosition.y
		);
	}

	void Widget::setExtent(const Coord2& _position, const Coord2& _size)
	{
		setFrame(
			_position.x,
			_position.y,
			_position.x + _size.x,
			_position.y + _size.y
		);
	}

	void Widget::setExtent(const RefFrame & _frame)
	{
		setFrame(_frame.left(), _frame.bottom(), _frame.right(), _frame.top());
	}

	void Widget::resize(float _deltaLeft, float _deltaRight, float _deltaBottom, float _deltaTop)
	{
		setFrame(
			left() + _deltaLeft,
			bottom() + _deltaBottom,
			right() + _deltaRight,
			top() + _deltaTop);
	}

	bool Widget::processInput(const struct MouseState& _mouseState)
	{
		bool cursorOnWidget = getRegion()->isMouseOver(_mouseState.position);

		// Any click on a different widget removes the keyboard focus.
		if(cursorOnWidget && _mouseState.anyButtonDown && GUIManager::getKeyboardFocussed() != this)
			GUIManager::setKeyboardFocus(nullptr);

		// Sticky focus remains untouched if enabled.
		if(GUIManager::getStickyMouseFocussed() == nullptr) {
			if(cursorOnWidget)
				GUIManager::setMouseFocus(this);
			// The current element has the focus but no reason to keep it.
			else if(GUIManager::hasMouseFocus(this))
				GUIManager::setMouseFocus(nullptr);
		}

		if(m_activeComponent)
		{
			bool ensureNextInput = false;
			bool ret = m_activeComponent->processInput(*this, _mouseState, cursorOnWidget, ensureNextInput);
			if(!ensureNextInput)
				m_activeComponent = nullptr;
			return ret;
		} else {
			// Let the behaviors check the input in order, once someone used the input stop and return.
			for(size_t i = 0; i < m_mouseInputSubcomponents.size(); ++i)
			{
				bool ensureNextInput = false;
				bool ret = m_mouseInputSubcomponents[i]->processInput(*this, _mouseState, cursorOnWidget, ensureNextInput);
				if(ensureNextInput && ret)
					m_activeComponent = m_mouseInputSubcomponents[i];
				if(ret) return true;
			}
		}

		if(GUIManager::getStickyMouseFocussed() == this)
			return true;
		return cursorOnWidget;
	}


	void Widget::addOnExtentChangeFunc(IExtentChangedReceiver* _receiver)
	{
		m_onExtentChangedFuncs.push_back(std::move(_receiver));
	}


	void Widget::removeOnExtentChangeFunc(IExtentChangedReceiver* _receiver)
	{
		std::remove_if(m_onExtentChangedFuncs.begin(), m_onExtentChangedFuncs.end(),
			[_receiver](IExtentChangedReceiver* _elem){ return _elem == _receiver; });
	}


	const IRegion * Widget::getRegion() const
	{
		if(m_region)
			return m_region;
		return this;
	}

	void Widget::setRegion(std::unique_ptr<IRegion> _region)
	{
		m_regionDeallocator = std::move(_region);
		m_region = m_regionDeallocator.get();
		m_region->attach(*this);
	}


	void Widget::registerMouseInputComponent(IMouseProcessAble * _component)
	{
		m_mouseInputSubcomponents.push_back(_component);
	}


	void Widget::onKeyboardFocus(bool _gotFocus)
	{
		if(m_onKeyboardFocus)
			m_onKeyboardFocus(this, _gotFocus);
	}

}} // namespace ca::gui