#pragma once

#include "widgets/widget.hpp"

namespace ca { namespace gui {

	const Widget* Widget::m_mouseFocus = nullptr;

	Widget::Widget(bool _anchorable, bool _clickable, bool _moveable, bool _resizeable, bool _inputReceivable, bool _focusable) : 
		m_anchorComponent(nullptr),
		m_clickComponent(nullptr),
		m_moveComponent(nullptr),
		m_resizeComponent(nullptr),
		m_inputReceivable(_inputReceivable),
		m_enabled(_inputReceivable),
		m_focusable(_focusable),
		m_visible(true),
		m_parent(nullptr)
	{
		if(_anchorable)
			m_anchorComponent = new Anchorable(&m_refFrame);
		if(_clickable)
			m_clickComponent = new Clickable(&m_refFrame);
		if(_moveable)
			m_moveComponent = new Moveable(&m_refFrame, m_anchorComponent);
		if(_resizeable)
			m_resizeComponent = new Resizeable(&m_refFrame, m_anchorComponent);
	}

	Widget::~Widget()
	{
		delete m_resizeComponent;
		delete m_moveComponent;
		delete m_clickComponent;
		delete m_anchorComponent;
	}

	void Widget::setSize(const Coord2& _size)
	{
		m_refFrame.sides[SIDE::RIGHT] = m_refFrame.left() + _size.x;
		m_refFrame.sides[SIDE::TOP] = m_refFrame.bottom() + _size.y;
		if(m_anchorComponent) m_anchorComponent->resetAnchors();
	}

	Coord2 Widget::getSize() const
	{
		return Coord2(m_refFrame.width(), m_refFrame.height());
	}

	void Widget::setPosition(const Coord2& _position)
	{
		m_refFrame.sides[SIDE::RIGHT]  = _position.x + m_refFrame.width();
		m_refFrame.sides[SIDE::TOP]    = _position.y + m_refFrame.height();
		m_refFrame.sides[SIDE::LEFT]   = _position.x;
		m_refFrame.sides[SIDE::BOTTOM] = _position.y;
		if(m_anchorComponent) m_anchorComponent->resetAnchors();
	}

	Coord2 Widget::getPosition() const
	{
		return Coord2(m_refFrame.left(), m_refFrame.bottom());
	}

	void Widget::setExtent(const Coord2& _position, const Coord2& _size)
	{
		m_refFrame.sides[SIDE::LEFT]   = _position.x;
		m_refFrame.sides[SIDE::BOTTOM] = _position.y;
		m_refFrame.sides[SIDE::RIGHT]  = _position.x + _size.x;
		m_refFrame.sides[SIDE::TOP]    = _position.y + _size.y;
		if(m_anchorComponent) m_anchorComponent->resetAnchors();
	}

	bool Widget::processInput(const struct MouseState& _mouseState)
	{
		if(m_clickComponent)
			if(m_clickComponent->processInput(_mouseState)) {
				m_mouseFocus = this;
				return true;
			}
		// Do resize before move, because it adds a little different behavior if the same input
		// is done close to the border.
		if(m_resizeComponent)
			if(m_resizeComponent->processInput(_mouseState)) {
				m_mouseFocus = this;
				return true;
			}
		if(m_moveComponent)
			if(m_moveComponent->processInput(_mouseState))
			{
				m_mouseFocus = this;
				return true;
			}
		if(m_refFrame.isMouseOver(_mouseState.position))
			m_mouseFocus = this;
		return false;
	}

}} // namespace ca::gui