#pragma once

#include "ca/gui/properties/resizeable.hpp"
#include "ca/gui/properties/refframe.hpp"
#include "ca/gui/properties/anchorable.hpp"
#include "ca/gui/backend/mouse.hpp"
#include "ca/gui/guimanager.hpp"

namespace ca { namespace gui {

	Resizeable::Resizeable(RefFrame* _selfFrame, Anchorable* _anchorable) :
		m_refFrame(_selfFrame),
		m_anchorable(_anchorable)
	{
	}

	static void chooseCursor(bool _flags[4])
	{
		if(_flags[0] && _flags[2])		GUIManager::setCursorType(CursorType::RESIZE_DUP);
		else if(_flags[0] && _flags[3])	GUIManager::setCursorType(CursorType::RESIZE_DDOWN);
		else if(_flags[1] && _flags[3])	GUIManager::setCursorType(CursorType::RESIZE_DUP);
		else if(_flags[1] && _flags[2])	GUIManager::setCursorType(CursorType::RESIZE_DDOWN);
		else if(_flags[0] || _flags[1]) GUIManager::setCursorType(CursorType::RESIZE_H);
		else if(_flags[2] || _flags[3]) GUIManager::setCursorType(CursorType::RESIZE_V);
	}

	bool Resizeable::processInput(const MouseState& _mouseState)
	{
		if(m_active)
		{
			chooseCursor(m_resizing);
			// If key was released stop floating
			if(_mouseState.buttons[0] == MouseState::PRESSED)
			{
				Coord2 deltaPos = _mouseState.deltaPos();
				if(m_resizing[SIDE::LEFT])		m_refFrame->sides[SIDE::LEFT] = ei::min(m_refFrame->sides[SIDE::LEFT] + deltaPos.x, m_refFrame->sides[SIDE::RIGHT]-1.0f);
				if(m_resizing[SIDE::RIGHT])		m_refFrame->sides[SIDE::RIGHT] = ei::max(m_refFrame->sides[SIDE::RIGHT] + deltaPos.x, m_refFrame->sides[SIDE::LEFT]+1.0f);
				if(m_resizing[SIDE::BOTTOM])	m_refFrame->sides[SIDE::BOTTOM] = ei::min(m_refFrame->sides[SIDE::BOTTOM] + deltaPos.y, m_refFrame->sides[SIDE::TOP]-1.0f);
				if(m_resizing[SIDE::TOP])		m_refFrame->sides[SIDE::TOP] = ei::max(m_refFrame->sides[SIDE::TOP] + deltaPos.y, m_refFrame->sides[SIDE::BOTTOM]+1.0f);
				// Make sure the anchoring does not reset the object
				if(m_anchorable)
					m_anchorable->resetAnchors();
			} else m_active = false;
			return true;
		} else {
			// Is the cursor within range?
			if(_mouseState.position.x >= m_refFrame->left()-2.0f
				&& _mouseState.position.x <= m_refFrame->right()+2.0f
				&& _mouseState.position.y >= m_refFrame->bottom()-2.0f
				&& _mouseState.position.y <= m_refFrame->top()+2.0f)
			{
				// Is the mouse cursor within a margin around the reference frame?
				m_resizing[SIDE::LEFT] = abs(m_refFrame->left() - _mouseState.position.x) <= 2.0f;
				m_resizing[SIDE::RIGHT] = abs(m_refFrame->right() - _mouseState.position.x) <= 2.0f;
				m_resizing[SIDE::BOTTOM] = abs(m_refFrame->bottom() - _mouseState.position.y) <= 2.0f;
				m_resizing[SIDE::TOP] = abs(m_refFrame->top() - _mouseState.position.y) <= 2.0f;
				bool anyFlag = (m_resizing[0] || m_resizing[1] || m_resizing[2] || m_resizing[3]);

				// Set a cursor if in margin
				chooseCursor(m_resizing);

				m_active = _mouseState.buttons[0] == MouseState::DOWN && anyFlag;
				return anyFlag;
			}
		}
		return false;
	}

}} // namespace ca::gui
