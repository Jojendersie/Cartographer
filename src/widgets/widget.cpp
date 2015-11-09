#pragma once

#include "widgets/widget.hpp"

namespace cag {

	Widget::Widget(bool _anchorable, bool _clickable, bool _moveable, bool _resizeable) : 
		m_anchorComponent(nullptr),
		m_clickComponent(nullptr),
		m_moveComponent(nullptr),
		m_resizeComponent(nullptr)
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

} // namespace cag