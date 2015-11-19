#pragma once

#include "widgets/button.hpp"
#include "cagui.hpp"
#include "rendering/theme.hpp"
#include "backend/renderbackend.hpp"

namespace cag {

	Button::Button() :
		Widget(true, true, false, false, true, true),
		m_backgroundTexture(0),
		m_iconTexture(0),
		m_textSize(0.0f),
		m_iconSize(0.0f)
	{
		m_enabled = true;
		m_clickComponent->setClickRegion(&m_refFrame, false);
	}

	void Button::setSize(const Coord2& _size)
	{
		m_refFrame.sides[SIDE::RIGHT] = m_refFrame.left() + _size.x;
		m_refFrame.sides[SIDE::TOP] = m_refFrame.right() + _size.y;
		m_anchorComponent->resetAnchors();
	}

	void Button::setPosition(const Coord2& _position)
	{
		m_refFrame.sides[SIDE::LEFT]   = _position.x;
		m_refFrame.sides[SIDE::BOTTOM] = _position.y;
		m_refFrame.sides[SIDE::RIGHT]  = _position.x + m_refFrame.width();
		m_refFrame.sides[SIDE::TOP]    = _position.y + m_refFrame.height();
		m_anchorComponent->resetAnchors();
	}

	void Button::setText(const char* _text)
	{
		m_text = _text;
		m_textSize;// TODO
	}

	void Button::setIcon(const char* _textureFile, SIDE::Val _side, const Coord2& _size, Coord _padding)
	{
		m_iconPos = _side;
		m_iconTexture = GUIManager::getRenderBackend()->getTexture(_textureFile);
		m_iconSize = _size;
	}

	void Button::setBackgroundTexture(const char* _textureFile)
	{
		m_backgroundTexture = GUIManager::getRenderBackend()->getTexture(_textureFile);
	}

	void Button::draw()
	{
		if(!isVisible()) return;

		// Set clip region to avoid overdraw (which should be impossible due to downsize?)
		bool vis = GUIManager::pushClipRegion(
			m_refFrame.left(), m_refFrame.right(),
			m_refFrame.bottom(), m_refFrame.top()
		);
		if(!vis) { GUIManager::popClipRegion(); return; }

		// Background
		bool mouseOver = GUIManager::hasFocus(this);
		if(m_backgroundTexture)
			GUIManager::getTheme()->drawImage(m_refFrame, m_backgroundTexture);
		else
			GUIManager::getTheme()->drawButton(m_refFrame, mouseOver);

		// Compute text and icon positions
		Coord2 iconPos, textPos;
		float downScale = 1.0f;
		if(m_iconPos == SIDE::LEFT || m_iconPos == SIDE::RIGHT)
		{
			float width = m_iconSize.x + m_textSize.x + m_iconPadding * 2.0f;
			float height = ei::max(m_iconSize.y, m_textSize.y);
			// autoscale if width or height is greater then the reference frame.
			downScale = ei::min(1.0f, m_refFrame.width() / width,
									  m_refFrame.height() / height);
			iconPos.y = textPos.y = (m_refFrame.bottom() + m_refFrame.top()) * 0.5f;
			textPos.y -= m_textSize.y * downScale * 0.5f;
			if(m_iconTexture && m_iconPos == SIDE::LEFT)
			{
				iconPos.x = m_refFrame.left() + m_iconPadding * downScale;
				textPos.x = m_refFrame.left() + (m_iconPadding * 2.0f + m_iconSize.x) * downScale;
			} else if(m_iconTexture && m_iconPos == SIDE::RIGHT) {
				iconPos.x = m_refFrame.right() - m_iconPadding * downScale;
				textPos.x = m_refFrame.right() - (m_iconPadding * 2.0f + m_iconSize.x + m_textSize.x) * downScale;
			} else // Center text
				textPos.x = (m_refFrame.left() + m_refFrame.right()) * 0.5f - m_textSize.x * downScale * 0.5f;
		} else {
			// TODO...
			iconPos.x = textPos.x = (m_refFrame.left() + m_refFrame.right()) * 0.5f;
		}

		// Icon
		if(m_iconTexture)
		{
			RefFrame rect;
			// TODO: rounding?
			rect.sides[SIDE::LEFT]   = iconPos.x - m_iconSize.x * downScale * 0.5f;
			rect.sides[SIDE::RIGHT]  = iconPos.x + m_iconSize.x * downScale * 0.5f;
			rect.sides[SIDE::BOTTOM] = iconPos.y - m_iconSize.y * downScale * 0.5f;
			rect.sides[SIDE::TOP]    = iconPos.y + m_iconSize.y * downScale * 0.5f;
			GUIManager::getTheme()->drawImage(rect, m_iconTexture);
		}

		// Text
		GUIManager::getTheme()->drawText(textPos, m_text.c_str(), mouseOver);
	}

	/*bool Button::processInput(const MouseState& _mouseState)
	{
		return m_clickComponent->processInput(_mouseState);
	}*/

} // namespace cag