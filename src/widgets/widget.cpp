#pragma once

#include "ca/gui/widgets/widget.hpp"
#include "ca/gui/guimanager.hpp"
#include "ca/gui/core/error.hpp"

namespace ca { namespace gui {

	Widget::Widget(bool _anchorable, bool _clickable, bool _moveable, bool _resizeable) : 
		m_anchorComponent(nullptr),
		m_clickComponent(nullptr),
		m_moveComponent(nullptr),
		m_resizeComponent(nullptr),
		m_anchorProvider(nullptr),
		m_enabled(_clickable || _moveable || _resizeable),
		m_keyboardFocusable(false),
		m_visible(true),
		m_parent(nullptr)
	{
		if(_anchorable)
			m_anchorComponent = std::make_unique<Anchorable>(&m_refFrame);
		if(_clickable)
			m_clickComponent = std::make_unique<Clickable>(&m_refFrame);
		if(_moveable)
			m_moveComponent = std::make_unique<Moveable>(&m_refFrame, m_anchorComponent.get());
		if(_resizeable)
			m_resizeComponent = std::make_unique<Resizeable>(&m_refFrame, m_anchorComponent.get());
	}

	void Widget::setSize(const Coord2& _size)
	{
		RefFrame oldFrame = m_refFrame;
		m_refFrame.sides[SIDE::RIGHT] = m_refFrame.left() + _size.x;
		m_refFrame.sides[SIDE::TOP] = m_refFrame.bottom() + _size.y;
		if(oldFrame != m_refFrame)
			onExtentChanged();
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
			onExtentChanged();
	}

	Coord2 Widget::getPosition() const
	{
		return Coord2(m_refFrame.left(), m_refFrame.bottom());
	}

	void Widget::setExtent(const Coord2& _position, const Coord2& _size)
	{
		RefFrame oldFrame = m_refFrame;
		m_refFrame.sides[SIDE::LEFT]   = _position.x;
		m_refFrame.sides[SIDE::BOTTOM] = _position.y;
		m_refFrame.sides[SIDE::RIGHT]  = _position.x + _size.x;
		m_refFrame.sides[SIDE::TOP]    = _position.y + _size.y;
		if(oldFrame != m_refFrame)
			onExtentChanged();
	}

	bool Widget::processInput(const struct MouseState& _mouseState)
	{
		if(m_clickComponent)
			if(m_clickComponent->processInput(_mouseState))
			{
				GUIManager::setMouseFocus(this, _mouseState.anyButtonDown || _mouseState.anyButtonPressed);
				return true;
			}// else GUIManager::setMouseFocus(nullptr, false);
		RefFrame oldFrame = m_refFrame;
		// Do resize before move, because it adds a little different behavior if the same input
		// is done close to the border.
		if(m_resizeComponent && !(m_moveComponent && m_moveComponent->isMoving()))
			if(m_resizeComponent->processInput(_mouseState))
			{
				GUIManager::setMouseFocus(this, (_mouseState.buttons[0] & (MouseState::DOWN | MouseState::PRESSED)) != 0);
				if(oldFrame != m_refFrame)
					onExtentChanged();
				return true;
			}
		if(m_moveComponent)
			if(m_moveComponent->processInput(_mouseState))
			{
				GUIManager::setCursorType(CursorType::MOVE);
				GUIManager::setMouseFocus(this, true);
				if(oldFrame != m_refFrame)
					onExtentChanged();
				return true;
			}
		if(GUIManager::getStickyMouseFocussed() == this)
			return true;
		if(m_refFrame.isMouseOver(_mouseState.position)) {
			GUIManager::setMouseFocus(this);
			// Clickables may define special areas, if not the reference frame defines the real
			// element area and the cursor is on this element.
			if(!m_clickComponent) return true;
		// The current element has the focus but now reason to keep it.
		} else if(GUIManager::hasMouseFocus(this))
			GUIManager::setMouseFocus(nullptr);
		return false;
	}

	void Widget::setAnchoring(SIDE::Val _side, AnchorPtr _anchor)
	{
		if(m_anchorComponent)
		{
			m_anchorComponent->setAnchor(_side, _anchor);
		} else
			error("Cannot set an anchor for a non-anchorable component!");
	}

	void Widget::setHorizontalAnchorMode(Anchorable::Mode _mode)
	{
		if(m_anchorComponent)
		{
			m_anchorComponent->setHorizontalAnchorMode(_mode);
		} else
			error("Cannot set horizontal anchor mode for a non-anchorable component!");
	}

	void Widget::setVerticalAnchorMode(Anchorable::Mode _mode)
	{
		if(m_anchorComponent)
		{
			m_anchorComponent->setVerticalAnchorMode(_mode);
		} else
			error("Cannot set vertical anchor mode for a non-anchorable component!");
	}

	void Widget::setAnchorModes(Anchorable::Mode _mode)
	{
		if(m_anchorComponent)
		{
			m_anchorComponent->setAnchorModes(_mode);
		} else
			error("Cannot set anchor modes for a non-anchorable component!");
	}

	void Widget::setAnchorModes(Anchorable::Mode _horizontalMode, Anchorable::Mode _verticalMode)
	{
		if(m_anchorComponent)
		{
			m_anchorComponent->setAnchorModes(_horizontalMode, _verticalMode);
		} else
			error("Cannot set anchor modes for a non-anchorable component!");
	}

	void Widget::autoAnchor(const IAnchorProvider* _anchorProvider)
	{
		if(m_anchorComponent)
			m_anchorComponent->autoAnchor(_anchorProvider);
	}

	void Widget::setAnchorProvider(std::unique_ptr<IAnchorProvider> _anchorProvider)
	{
		m_anchorProvider = move(_anchorProvider);
		if(m_anchorProvider)
			m_anchorProvider->recomputeAnchors(m_refFrame);
	}

	void Widget::setClickable(bool _enable)
	{
		if(_enable && !m_clickComponent)
			m_clickComponent = std::make_unique<Clickable>(&m_refFrame);
		
		
	}

	void Widget::setAnchorable(bool _enable)
	{
		if(_enable && !m_anchorComponent) {
			m_anchorComponent = std::make_unique<Anchorable>(&m_refFrame);
			if(m_moveComponent) m_moveComponent->registerAnchorCompoentent(m_anchorComponent.get());
			if(m_resizeComponent) m_resizeComponent->registerAnchorCompoentent(m_anchorComponent.get());
		} else if(!_enable && m_anchorComponent) {
			if(m_moveComponent) m_moveComponent->registerAnchorCompoentent(nullptr);
			if(m_resizeComponent) m_resizeComponent->registerAnchorCompoentent(nullptr);
			m_anchorComponent = nullptr;
		}
	}

	void Widget::setMoveable(bool _enable)
	{
		if(_enable && !m_moveComponent)
			m_moveComponent = std::make_unique<Moveable>(&m_refFrame, m_anchorComponent.get());
		else m_moveComponent = nullptr;
	}

	void Widget::setResizeable(bool _enable)
	{
		if(_enable && !m_resizeComponent)
			m_resizeComponent = std::make_unique<Resizeable>(&m_refFrame, m_anchorComponent.get());
		else m_resizeComponent = nullptr;
	}

	void Widget::refitToAnchors()
	{
		if(m_anchorComponent)
			if(m_anchorComponent->refitToAnchors())
				onExtentChanged();
	}

	void Widget::onExtentChanged()
	{
		// Make sure the anchoring does not reset the object to its previous position.
		if(m_anchorComponent)
			m_anchorComponent->resetAnchors();
		if(m_anchorProvider)
			m_anchorProvider->recomputeAnchors( m_refFrame );
	}

}} // namespace ca::gui