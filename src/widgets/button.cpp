#pragma once

#include "widgets/button.hpp"
#include "cagui.hpp"
#include "rendering/theme.hpp"
#include "backend/renderbackend.hpp"

namespace cag {

	Button::Button() :
		Widget(true, true, false, false),
		m_backgroundTexture(0),
		m_iconTexture(0),
		m_textSize(0.0f),
		m_iconSize(0.0f)
	{
		m_active = true;
		m_clickComponent->setClickRegion(&m_refFrame, false);
	}

	void Button::setSize(const Coord& _size)
	{
		m_refFrame.right = m_refFrame.left + _size.x;
		m_refFrame.top = m_refFrame.bottom + _size.y;
		m_anchorComponent->resetAnchors();
	}

	void Button::setPosition(const Coord& _position)
	{
		m_refFrame.right = _position.x + (m_refFrame.right - m_refFrame.left);
		m_refFrame.right = _position.y + (m_refFrame.top - m_refFrame.bottom);
		m_refFrame.left = _position.x;
		m_refFrame.bottom = _position.y;
		m_anchorComponent->resetAnchors();
	}

	void Button::setText(const char* _text)
	{
		m_text = _text;
		m_textSize;// TODO
	}

	void Button::setIcon(const char* _textureFile, SIDE _side, const Coord& _size, Range _padding)
	{
		m_iconPos = _side;
		m_iconTexture = GUIManagar::getRenderBackend()->getTexture(_textureFile);
		m_iconSize = _size;
	}

	void Button::setBackgroundTexture(const char* _textureFile)
	{
		m_backgroundTexture = GUIManagar::getRenderBackend()->getTexture(_textureFile);
	}

	void Button::draw()
	{
		// Set clip region to avoid overdraw (which should be impossible due to downsize?)
		GUIManagar::getRenderBackend()->setClippingRegion(
			ei::round(m_refFrame.left), ei::round(m_refFrame.right),
			ei::round(m_refFrame.bottom), ei::round(m_refFrame.top)
		);

		// Background
		if(m_backgroundTexture)
			GUIManagar::getTheme()->drawImage(m_refFrame, m_backgroundTexture);
		else
			GUIManagar::getTheme()->drawButton(m_refFrame, m_mouseOver);

		// Compute text and icon positions
		Coord iconPos, textPos;
		float downScale = 1.0f;
		if(m_iconPos == SIDE::LEFT || m_iconPos == SIDE::RIGHT)
		{
			float width = m_iconSize.x + m_textSize.x + m_iconPadding * 2.0f;
			float height = ei::max(m_iconSize.y, m_textSize.y);
			// autoscale if width or height is greater then the reference frame.
			downScale = ei::min(1.0f, (m_refFrame.right - m_refFrame.left) / width,
									  (m_refFrame.top - m_refFrame.bottom) / height);
			iconPos.y = textPos.y = (m_refFrame.bottom + m_refFrame.top) * 0.5f;
			textPos.y -= m_textSize.y * downScale * 0.5f;
			if(m_iconTexture && m_iconPos == SIDE::LEFT)
			{
				iconPos.x = m_refFrame.left + m_iconPadding * downScale;
				textPos.x = m_refFrame.left + (m_iconPadding * 2.0f + m_iconSize.x) * downScale;
			} else if(m_iconTexture && m_iconPos == SIDE::RIGHT) {
				iconPos.x = m_refFrame.right - m_iconPadding * downScale;
				textPos.x = m_refFrame.right - (m_iconPadding * 2.0f + m_iconSize.x + m_textSize.x) * downScale;
			} else // Center text
				textPos.x = (m_refFrame.left + m_refFrame.right) * 0.5f - m_textSize.x * downScale * 0.5f;
		} else {
			// TODO...
			iconPos.x = textPos.x = (m_refFrame.left + m_refFrame.right) * 0.5f;
		}

		// Icon
		if(m_iconTexture)
		{
			RefFrame rect;
			// TODO: rounding?
			rect.left = iconPos.x - m_iconSize.x * downScale * 0.5f;
			rect.right = iconPos.x + m_iconSize.x * downScale * 0.5f;
			rect.bottom = iconPos.y - m_iconSize.y * downScale * 0.5f;
			rect.top = iconPos.y + m_iconSize.y * downScale * 0.5f;
			GUIManagar::getTheme()->drawImage(rect, m_iconTexture);
		}

		// Text
		GUIManagar::getTheme()->drawText(textPos, m_text.c_str(), m_mouseOver);
	}

	void Button::processInput(const MouseState& _mouseState)
	{
		m_clickComponent->processInput(_mouseState);
	}

} // namespace cag