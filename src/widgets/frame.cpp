#pragma once

#include "widgets/frame.hpp"
#include "backend/mouse.hpp"
#include "backend/renderbackend.hpp"
#include "cagui.hpp"
#include "rendering/theme.hpp"

namespace cag {

	Frame::Frame(bool _anchorable, bool _clickable, bool _moveable, bool _resizeable) :
		Widget(_anchorable, _clickable, _moveable, _resizeable, true, true),
		m_opacity(0.0f),
		m_texture(0)
	{
	}

	void Frame::draw()
	{
		if(m_visible)
		{
			// Set clipping region for this and all subelements
			bool vis = GUIManager::pushClipRegion(
				m_refFrame.left(), m_refFrame.right(),
				m_refFrame.bottom(), m_refFrame.top()
			);
			if(vis)
			{
				// Draw the frame background
				if(m_texture)
					GUIManager::getTheme()->drawImage(m_refFrame, m_texture, m_opacity);
				else
					GUIManager::getTheme()->drawBackgroundArea(m_opacity);

				for(const auto& i : m_passiveChildren)
					i->draw();
				// Draw all active components in reverse order (the first one has the focus and should
				// be on top)
				for(auto i = m_activeChildren.rbegin(); i != m_activeChildren.rend(); ++i)
					(*i)->draw();
			}

			GUIManager::popClipRegion();
		}
	}

	void Frame::add(WidgetPtr _widget)
	{
		_widget->m_parent = this;
		if(_widget->isInputReceivable())
			m_activeChildren.push_back(move(_widget));
		else
			m_passiveChildren.push_back(move(_widget));
	}

	void Frame::remove(WidgetPtr _widget)
	{
		// Search linearly in one of the lists. Since isInputReceivable() is guaranteed to be const
		// for the lifetime of the object it cannot switch between lists.
		if(_widget->isInputReceivable())
		{
			for(auto it = m_activeChildren.begin(); it != m_activeChildren.end(); ++it)
				if(*it == _widget)
				{
					m_activeChildren.erase(it);
					return;
				}
		} else {
			for(auto it = m_passiveChildren.begin(); it != m_passiveChildren.end(); ++it)
				if(*it == _widget)
				{
					m_passiveChildren.erase(it);
					return;
				}
		}
	}

	bool Frame::processInput(const MouseState& _mouseState)
	{
		// Component disabled?
		if(!isEnabled() || !isVisible()) return false;
		// Forward to subelements
		for(size_t i = 0; i < m_activeChildren.size(); ++i)
		{
			WidgetPtr& e = m_activeChildren[i];
			if(e->isEnabled() && e->processInput(_mouseState))
			{
				// The one who took the input gets the focus
				if(e->isFocusable()) focusOn(i);
				return true;
			}
		}
		// Input was not consumed by an element.
		// If there are properties try them.
		return Widget::processInput(_mouseState);
	}

	bool Frame::isChildFocused(const Widget* _child) const
	{
		// TODO: Assert m_activeChildren.size() > 0
		return m_activeChildren[0].get() == _child && _child->isFocusable();
	}

	void Frame::setBackground(const char* _imageFile, float _opacity)
	{
		m_texture = GUIManager::getRenderBackend()->getTexture(_imageFile);
		m_opacity = _opacity;
	}

	void Frame::setBackgroundOpacity(float _opacity)
	{
		m_opacity = _opacity;
	}

	void Frame::focusOn(size_t _index)
	{
		if(_index > 0)
		{
			// Move to front and keep relative order of everything else
			std::rotate( m_activeChildren.begin(),
				m_activeChildren.begin() + _index,
				m_activeChildren.begin() + _index );
		}
	}

} // namespace cag