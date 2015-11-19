#pragma once

#include "widgets/widget.hpp"

namespace cag {

	Widget::Widget(bool _anchorable, bool _clickable, bool _moveable, bool _resizeable, bool _inputReceivable, bool _focusable) : 
		m_anchorComponent(nullptr),
		m_clickComponent(nullptr),
		m_moveComponent(nullptr),
		m_resizeComponent(nullptr),
		m_inputReceivable(_inputReceivable),
		m_active(_inputReceivable),
		m_focusable(_focusable)
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

	bool Widget::processInput(const struct MouseState& _mouseState)
	{
		if(m_clickComponent)
			if(m_clickComponent->processInput(_mouseState))
				return true;
		// Do resize before move, because it adds a little different behavior if the same input
		// is done close to the border.
		if(m_resizeComponent)
			if(m_resizeComponent->processInput(_mouseState))
				return true;
		if(m_moveComponent)
			if(m_moveComponent->processInput(_mouseState))
				return true;
		return false;
	}

} // namespace cag