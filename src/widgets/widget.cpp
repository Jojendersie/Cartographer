#pragma once

#include "widgets/widget.hpp"
#include "guimanager.hpp"
#include "ca/gui/core/error.hpp"

namespace ca { namespace gui {

	Widget::Widget(bool _anchorable, bool _clickable, bool _moveable, bool _resizeable, bool _inputReceivable, bool _focusable) : 
		m_anchorComponent(nullptr),
		m_clickComponent(nullptr),
		m_moveComponent(nullptr),
		m_resizeComponent(nullptr),
		m_anchorProvider(nullptr),
		m_inputReceivable(_inputReceivable),
		m_enabled(_inputReceivable),
		m_focusable(_focusable),
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
			if(m_clickComponent->processInput(_mouseState)) {
				GUIManager::setMouseFocus(this);
				return true;
			}
		RefFrame oldFrame = m_refFrame;
		// Do resize before move, because it adds a little different behavior if the same input
		// is done close to the border.
		if(m_resizeComponent)
			if(m_resizeComponent->processInput(_mouseState)) {
				GUIManager::setMouseFocus(this, true);
				if(oldFrame != m_refFrame)
					onExtentChanged();
				return true;
			}
		if(m_moveComponent)
			if(m_moveComponent->processInput(_mouseState))
			{
				GUIManager::setMouseFocus(this, true);
				if(oldFrame != m_refFrame)
					onExtentChanged();
				return true;
			}
		if(m_refFrame.isMouseOver(_mouseState.position))
			GUIManager::setMouseFocus(this);
		// The current element has the focus but now reson to keep it.
		else if(GUIManager::hasMouseFocus(this))
			GUIManager::setMouseFocus(nullptr);
		return false;
	}

	void Widget::setAnchor(SIDE::Val _side, AnchorPtr _anchor)
	{
		if(m_anchorComponent)
		{
			m_anchorComponent->setAnchor(_side, _anchor);
		} else
			error("Cannot set an anchor for a non-anchorable component!");
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
			m_anchorProvider->replaceAnchors( m_refFrame );
	}

}} // namespace ca::gui