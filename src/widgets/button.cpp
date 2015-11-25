#pragma once

#include "widgets/button.hpp"
#include "guimanager.hpp"
#include "rendering/theme.hpp"
#include "backend/renderbackend.hpp"

namespace ca { namespace gui {

	Button::Button() :
		Widget(true, true, false, false, true, false),
		m_backgroundTexture(0),
		m_iconTexture(0),
		m_textSize(0.0f),
		m_iconSize(0.0f),
		m_iconPos(SIDE::LEFT)
	{
		m_enabled = true;
		m_clickComponent->setClickRegion(&m_refFrame, false);
	}

	void Button::setText(const char* _text)
	{
		m_text = _text;
		m_textSize;// TODO
	}

	void Button::setIcon(const char* _textureFile, SIDE::Val _side, const Coord2& _size, bool _smooth, Coord _padding)
	{
		m_iconPos = _side;
		m_iconTexture = GUIManager::renderBackend().getTexture(_textureFile, _smooth);
		m_iconSize = _size;
	}

	void Button::setBackgroundTexture(const char* _textureFile, bool _smooth)
	{
		m_backgroundTexture = GUIManager::renderBackend().getTexture(_textureFile, _smooth);
	}

	void Button::draw()
	{
		if(!isVisible()) return;
		if(GUIManager::isClipped( m_refFrame )) return;

		// Background
		bool mouseOver = m_clickComponent->isOnClickRegion( GUIManager::getMouseState().position );
		if(m_backgroundTexture)
			GUIManager::theme().drawImage(m_refFrame, m_backgroundTexture);
		else
			GUIManager::theme().drawButton(m_refFrame, mouseOver);

		// Compute text and icon positions
		Coord2 iconPos, textPos;
		float downScale = 1.0f;
		if(!m_iconTexture)
		{
			textPos.x = (m_refFrame.left() + m_refFrame.right()) * 0.5f;
			textPos.y = (m_refFrame.bottom() + m_refFrame.top()) * 0.5f;
			textPos -= m_textSize * downScale * 0.5f;
		} else if(m_iconPos == SIDE::LEFT || m_iconPos == SIDE::RIGHT)
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
			GUIManager::theme().drawImage(rect, m_iconTexture);
		}

		// Text
		GUIManager::theme().drawText(textPos, m_text.c_str(), mouseOver);
	}

	/*bool Button::processInput(const MouseState& _mouseState)
	{
		return m_clickComponent->processInput(_mouseState);
	}*/

}} // namespace ca::gui