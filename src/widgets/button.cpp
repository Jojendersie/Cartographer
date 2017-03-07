#pragma once

#include "ca/gui/widgets/button.hpp"
#include "ca/gui/guimanager.hpp"
#include "ca/gui/rendering/theme.hpp"
#include "ca/gui/backend/renderbackend.hpp"

namespace ca { namespace gui {

	Button::Button() :
		Widget(true, true, false, false),
		m_textSize(0.0f),
		m_iconPos(SIDE::LEFT),
		m_iconSize(0.0f),
		m_iconPadding(2.0f),
		m_iconTexture(0),
		m_backgroundTexture(0)
	{
		Widget::setKeyboardFocusable(true);
		m_enabled = true;
	}

	void Button::setText(const char* _text)
	{
		m_text = _text;
		m_textSize = GUIManager::theme().getTextBB(Coord2(0.0f), _text, 1.0f).max;
	}

	void Button::setIcon(const char* _textureFile, SIDE::Val _side, const Coord2& _size, bool _smooth, Coord _padding)
	{
		m_iconPos = _side;
		m_iconTexture = GUIManager::renderBackend().getTexture(_textureFile, _smooth);
		m_iconSize = _size;
		m_iconPadding = _padding;
	}

	void Button::setBackgroundTexture(const char* _textureFile, const char* _hoverTextureFile, const char* _downTextureFile, bool _smooth)
	{
		m_backgroundTexture = GUIManager::renderBackend().getTexture(_textureFile, _smooth);
		if(_hoverTextureFile) m_hoverTexture = GUIManager::renderBackend().getTexture(_hoverTextureFile, _smooth);
		if(_downTextureFile) m_downTexture = GUIManager::renderBackend().getTexture(_downTextureFile, _smooth);
	}

	void Button::draw() const
	{
		// Background
		bool mouseOver = isMouseOver();
		if(m_backgroundTexture) {
			if(m_clickComponent->isAnyButtonDown() && m_downTexture)
				GUIManager::theme().drawImage(m_refFrame, m_downTexture);
			else if(mouseOver && m_hoverTexture)
				GUIManager::theme().drawImage(m_refFrame, m_hoverTexture);
			else
				GUIManager::theme().drawImage(m_refFrame, m_backgroundTexture);
		} else
			GUIManager::theme().drawButton(m_refFrame, mouseOver, m_clickComponent->isAnyButtonDown());

		// Compute text and icon positions
		Coord2 iconPos, textPos;
		Coord2 effectiveTextSize = m_textSize * m_relativeTextSize;
		float downScale = 1.0f;
		if(!m_iconTexture)
		{
			// autoscale if width or height is greater then the reference frame.
			downScale = ei::min(1.0f, m_refFrame.width() / (effectiveTextSize.x),
									  m_refFrame.height() / (effectiveTextSize.y));
			textPos.x = (m_refFrame.left() + m_refFrame.right()) * 0.5f;
			textPos.y = (m_refFrame.bottom() + m_refFrame.top()) * 0.5f;
			textPos -= effectiveTextSize * downScale * 0.5f;
		} else if(m_text.empty()) {
			// autoscale if width or height is greater then the reference frame.
			downScale = ei::min(1.0f, m_refFrame.width() / (m_iconSize.x + m_iconPadding * 2.0f),
									  m_refFrame.height() / (m_iconSize.y + m_iconPadding * 2.0f));
			iconPos.x = (m_refFrame.left() + m_refFrame.right()) * 0.5f;
			iconPos.y = (m_refFrame.bottom() + m_refFrame.top()) * 0.5f;
			iconPos -= m_iconSize * downScale * 0.5f;
		} else if(m_iconPos == SIDE::LEFT || m_iconPos == SIDE::RIGHT)
		{
			float width = m_iconSize.x + effectiveTextSize.x + m_iconPadding * 2.0f;
			float height = ei::max(m_iconSize.y, effectiveTextSize.y) + m_iconPadding * 2.0f;
			// autoscale if width or height is greater then the reference frame.
			downScale = ei::min(1.0f, m_refFrame.width() / width,
									  m_refFrame.height() / height);
			iconPos.y = textPos.y = (m_refFrame.bottom() + m_refFrame.top()) * 0.5f;
			textPos.y -= effectiveTextSize.y * downScale * 0.5f;
			iconPos.y -= m_iconSize.y * downScale * 0.5f;
			if(m_iconPos == SIDE::LEFT)
			{
				iconPos.x = m_refFrame.left() + m_iconPadding * downScale;
				textPos.x = m_refFrame.left() + (m_iconPadding * 2.0f + m_iconSize.x) * downScale;
			} else {
				iconPos.x = m_refFrame.right() - (m_iconPadding + m_iconSize.x) * downScale;
				textPos.x = m_refFrame.right() - (m_iconPadding * 2.0f + m_iconSize.x + effectiveTextSize.x) * downScale;
			}
		} else {
			float width = ei::max(m_iconSize.x, effectiveTextSize.x) + m_iconPadding * 2.0f;
			float height = m_iconSize.y + effectiveTextSize.y + m_iconPadding * 3.0f;
			// autoscale if width or height is greater then the reference frame.
			downScale = ei::min(1.0f, m_refFrame.width() / width,
									  m_refFrame.height() / height);
			iconPos.x = textPos.x = (m_refFrame.left() + m_refFrame.right()) * 0.5f;
			textPos.x -= effectiveTextSize.x * downScale * 0.5f;
			iconPos.x -= m_iconSize.x * downScale * 0.5f;
			if(m_iconPos == SIDE::BOTTOM)
			{
				iconPos.y = m_refFrame.bottom() + m_iconPadding * downScale;
				textPos.y = m_refFrame.bottom() + (m_iconPadding * 2.0f + m_iconSize.y) * downScale;
			} else {
				iconPos.y = m_refFrame.top() - (m_iconPadding + m_iconSize.y) * downScale;
				textPos.y = m_refFrame.top() - (m_iconPadding * 2.0f + m_iconSize.y + effectiveTextSize.y) * downScale;
			}
		}

		// Icon
		if(m_iconTexture)
		{
			RefFrame rect;
			rect.sides[SIDE::LEFT]   = floorf(iconPos.x);
			rect.sides[SIDE::RIGHT]  = floorf(iconPos.x + m_iconSize.x * downScale);
			rect.sides[SIDE::BOTTOM] = floorf(iconPos.y);
			rect.sides[SIDE::TOP]    = floorf(iconPos.y + m_iconSize.y * downScale);
			GUIManager::theme().drawImage(rect, m_iconTexture);
		}

		// Text
		GUIManager::theme().drawText(textPos, m_text.c_str(), downScale * m_relativeTextSize, mouseOver);
	}

}} // namespace ca::gui