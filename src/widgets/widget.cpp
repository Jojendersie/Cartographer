#include "ca/gui/widgets/widget.hpp"
#include "ca/gui/guimanager.hpp"
#include <ca/pa/log.hpp>

namespace ca { namespace gui {

	using namespace pa;

	Widget::Widget() : 
		Anchorable(&m_refFrame),
		m_enabled(true),
		m_keyboardFocusable(false),
		m_visible(true),
		m_region(nullptr),
		m_anchorProvider(nullptr),
		m_activeComponent(nullptr),
		m_parent(nullptr)
	{
	}

	Widget::~Widget()
	{
		if(GUIManager::hasMouseFocus(this))
			GUIManager::setMouseFocus(nullptr);
		if(GUIManager::hasKeyboardFocus(this))
			GUIManager::setKeyboardFocus(nullptr);
	}

	void Widget::setSize(const Coord2& _size)
	{
		RefFrame oldFrame = m_refFrame;
		m_refFrame.sides[SIDE::RIGHT] = m_refFrame.left() + _size.x;
		m_refFrame.sides[SIDE::TOP] = m_refFrame.bottom() + _size.y;
		if(oldFrame != m_refFrame)
			onExtentChanged(false, true);
	}

	Coord2 Widget::getSize() const
	{
		return Coord2(m_refFrame.width(), m_refFrame.height());
	}

	void Widget::setPosition(const Coord2& _position)
	{
		RefFrame oldFrame = m_refFrame;
		m_refFrame.sides[SIDE::RIGHT]  = _position.x + m_refFrame.width();
		m_refFrame.sides[SIDE::TOP]    = _position.y + m_refFrame.height();
		m_refFrame.sides[SIDE::LEFT]   = _position.x;
		m_refFrame.sides[SIDE::BOTTOM] = _position.y;
		if(oldFrame != m_refFrame)
			onExtentChanged(true, false);
	}

	Coord2 Widget::getPosition() const
	{
		return m_refFrame.position();
	}

	void Widget::move(const Coord2 & _deltaPosition)
	{
		if(_deltaPosition != Coord2(0.0f))
		{
			m_refFrame.sides[SIDE::LEFT] += _deltaPosition.x;
			m_refFrame.sides[SIDE::RIGHT] += _deltaPosition.x;
			m_refFrame.sides[SIDE::BOTTOM] += _deltaPosition.y;
			m_refFrame.sides[SIDE::TOP] += _deltaPosition.y;
			onExtentChanged(true, false);
		}
	}

	void Widget::setExtent(const Coord2& _position, const Coord2& _size)
	{
		bool posChanged = _position != m_refFrame.position();
		bool sizeChanged = _size != m_refFrame.size();
		if(posChanged || sizeChanged)
		{
			m_refFrame.sides[SIDE::LEFT]   = _position.x;
			m_refFrame.sides[SIDE::BOTTOM] = _position.y;
			m_refFrame.sides[SIDE::RIGHT]  = _position.x + _size.x;
			m_refFrame.sides[SIDE::TOP]    = _position.y + _size.y;
			onExtentChanged(posChanged, sizeChanged);
		}
	}

	void Widget::resize(float _deltaLeft, float _deltaRight, float _deltaBottom, float _deltaTop)
	{
		RefFrame oldFrame = m_refFrame;
		m_refFrame.sides[SIDE::LEFT] = ei::min(m_refFrame.left() + _deltaLeft, m_refFrame.right() - 1.0f);
		m_refFrame.sides[SIDE::RIGHT] = ei::max(m_refFrame.right() + _deltaRight, m_refFrame.left() + 1.0f);
		m_refFrame.sides[SIDE::BOTTOM] = ei::min(m_refFrame.bottom() + _deltaBottom, m_refFrame.top() - 1.0f);
		m_refFrame.sides[SIDE::TOP] = ei::max(m_refFrame.top() + _deltaTop, m_refFrame.bottom() + 1.0f);
		if(oldFrame != m_refFrame)
		{
			onExtentChanged(false, true);
		}
	}

	bool Widget::processInput(const struct MouseState& _mouseState)
	{
		bool cursorOnWidget = getRegion()->isMouseOver(_mouseState.position);

		// Any click on a different widget removes the keyboard focus.
		if(cursorOnWidget && _mouseState.anyButtonDown && GUIManager::getKeyboardFocussed() != this)
			GUIManager::setKeyboardFocus(nullptr);

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
		if(cursorOnWidget) {
			GUIManager::setMouseFocus(this);
			return true;
		// The current element has the focus but no reason to keep it.
		} else if(GUIManager::hasMouseFocus(this))
			GUIManager::setMouseFocus(nullptr);
		return false;
	}

	bool Widget::isMouseOver() const
	{
		return GUIManager::hasMouseFocus(this)
			&& getRegion()->isMouseOver(GUIManager::getMouseState().position);
	}

	void Widget::setAnchorProvider(std::unique_ptr<IAnchorProvider> _anchorProvider)
	{
		m_anchorProvider = std::move(_anchorProvider);
		if(m_anchorProvider)
			m_anchorProvider->recomputeAnchors(m_refFrame);
	}

	const IRegion * Widget::getRegion() const
	{
		if(m_region)
			return m_region;
		return &m_refFrame;
	}

	void Widget::setRegion(std::unique_ptr<IRegion> _region)
	{
		m_regionDeallocator = std::move(_region);
		m_region = m_regionDeallocator.get();
		m_region->attach(m_refFrame);
	}


	void Widget::refitToAnchors()
	{
		if(Anchorable::refitToAnchors())
			onExtentChanged(true, true);
	}

	void Widget::registerMouseInputComponent(IMouseProcessAble * _component)
	{
		m_mouseInputSubcomponents.push_back(_component);
	}

	void Widget::onExtentChanged(bool _positionChanged, bool _sizeChanged)
	{
		// Make sure the anchoring does not reset the object to its previous position.
		if(isAnchoringEnabled())
			resetAnchors();
		if(m_anchorProvider)
			m_anchorProvider->recomputeAnchors( m_refFrame );
	}

	void Widget::onKeyboardFocus(bool _gotFocus)
	{
		if(m_onKeyboardFocus)
			m_onKeyboardFocus(this, _gotFocus);
	}

}} // namespace ca::gui