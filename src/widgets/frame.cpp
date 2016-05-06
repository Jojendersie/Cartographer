#pragma once

#include "widgets/frame.hpp"
#include "backend/mouse.hpp"
#include "backend/renderbackend.hpp"
#include "guimanager.hpp"
#include "rendering/theme.hpp"

namespace ca { namespace gui {

	Frame::Frame(bool _anchorable, bool _clickable, bool _moveable, bool _resizeable) :
		m_opacity(1.0f),
		m_texture(0)
	{
		Widget::setAnchorable(_anchorable);
		Widget::setClickable(_clickable);
		Widget::setMoveable(_moveable);
		Widget::setResizeable(_resizeable);
	}

	Frame::~Frame()
	{
	}

	void Frame::draw()
	{
		if(m_visible)
		{
			// Set clipping region for this and all subelements
			bool vis = GUIManager::pushClipRegion(m_refFrame);
			if(vis)
			{
				// Draw the frame background
				if(m_texture)
					GUIManager::theme().drawImage(m_refFrame, m_texture, m_opacity);
				else
					GUIManager::theme().drawBackgroundArea(m_refFrame, m_opacity);

				// Draw all contained children of the group
				Group::draw();
			}

			GUIManager::popClipRegion();
		}
	}

	bool Frame::processInput(const MouseState& _mouseState)
	{
		// Component disabled?
		if(!isEnabled() || !isVisible()) return false;
		if(Group::processInput(_mouseState)) return true;
		// Input was not consumed by an element.
		// If there are properties try them. Since move and resize require input handling outside
		// the reference frame this is not inside the isMouseOver-block.
		return Widget::processInput(_mouseState);
	}

	void Frame::setBackground(const char* _imageFile, bool _smooth, float _opacity)
	{
		m_texture = GUIManager::renderBackend().getTexture(_imageFile, _smooth);
		m_opacity = _opacity;
	}

	void Frame::setBackgroundOpacity(float _opacity)
	{
		m_opacity = _opacity;
	}


}} // namespace ca::gui