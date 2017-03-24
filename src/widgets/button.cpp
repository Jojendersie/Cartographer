#pragma once

#include "ca/gui/widgets/button.hpp"
#include "ca/gui/guimanager.hpp"
#include "ca/gui/rendering/theme.hpp"
#include "ca/gui/backend/renderbackend.hpp"

namespace ca { namespace gui {

	Button::Button() :
		Widget(true, true, false, false),
		m_textSize(0.0f),
		m_iconPlacement(SIDE::LEFT),
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
		recomputeIconNTextPositions();
	}

	void Button::setIcon(const char* _textureFile, SIDE::Val _side, const Coord2& _size, bool _smooth, Coord _padding)
	{
		m_iconPlacement = _side;
		m_iconTexture = GUIManager::renderBackend().getTexture(_textureFile, _smooth);
		m_iconSize = _size;
		m_iconPadding = _padding;
		recomputeIconNTextPositions();
	}

	void Button::setIcon(const char * _textureFile, const Coord2 & _position, const Coord2 & _size, bool _smooth)
	{
		m_iconPlacement = SIDE::CENTER; // Mark absolute positions with an invalid value
		m_iconTexture = GUIManager::renderBackend().getTexture(_textureFile, _smooth);
		m_iconSize = _size;
		m_iconPadding = 0.0f;
		m_iconPos = _position;
		recomputeIconNTextPositions();
	}

	void Button::setBackgroundTexture(const char* _textureFile, const char* _hoverTextureFile, const char* _downTextureFile, bool _smooth)
	{
		m_backgroundTexture = GUIManager::renderBackend().getTexture(_textureFile, _smooth);
		if(_hoverTextureFile) m_hoverTexture = GUIManager::renderBackend().getTexture(_hoverTextureFile, _smooth);
		if(_downTextureFile) m_downTexture = GUIManager::renderBackend().getTexture(_downTextureFile, _smooth);
	}

	void Button::recomputeIconNTextPositions()
	{
		Coord2 effectiveTextSize = m_textSize * m_relativeTextSize;
		if(!m_iconTexture || isIconPlacementAbsolute())
		{
			// autoscale if width or height is greater then the reference frame.
			m_downScale = ei::min(1.0f, m_refFrame.width() / (effectiveTextSize.x),
				m_refFrame.height() / (effectiveTextSize.y));
			m_textPos.x = (m_refFrame.left() + m_refFrame.right()) * 0.5f;
			m_textPos.y = (m_refFrame.bottom() + m_refFrame.top()) * 0.5f;
			m_textPos -= effectiveTextSize * m_downScale * 0.5f;
		} else if(m_text.empty()) {
			// autoscale if width or height is greater then the reference frame.
			m_downScale = ei::min(1.0f, m_refFrame.width() / (m_iconSize.x + m_iconPadding * 2.0f),
				m_refFrame.height() / (m_iconSize.y + m_iconPadding * 2.0f));
			m_iconPos.x = (m_refFrame.left() + m_refFrame.right()) * 0.5f;
			m_iconPos.y = (m_refFrame.bottom() + m_refFrame.top()) * 0.5f;
			m_iconPos -= m_iconSize * m_downScale * 0.5f;
		} else if(m_iconPlacement == SIDE::LEFT || m_iconPlacement == SIDE::RIGHT)
		{
			float width = m_iconSize.x + effectiveTextSize.x + m_iconPadding * 2.0f;
			float height = ei::max(m_iconSize.y, effectiveTextSize.y) + m_iconPadding * 2.0f;
			// autoscale if width or height is greater then the reference frame.
			m_downScale = ei::min(1.0f, m_refFrame.width() / width,
				m_refFrame.height() / height);
			m_iconPos.y = m_textPos.y = (m_refFrame.bottom() + m_refFrame.top()) * 0.5f;
			m_textPos.y -= effectiveTextSize.y * m_downScale * 0.5f;
			m_iconPos.y -= m_iconSize.y * m_downScale * 0.5f;
			if(m_iconPlacement == SIDE::LEFT)
			{
				m_iconPos.x = m_refFrame.left() + m_iconPadding * m_downScale;
				m_textPos.x = m_refFrame.left() + (m_iconPadding * 2.0f + m_iconSize.x) * m_downScale;
			} else {
				m_iconPos.x = m_refFrame.right() - (m_iconPadding + m_iconSize.x) * m_downScale;
				m_textPos.x = m_refFrame.right() - (m_iconPadding * 2.0f + m_iconSize.x + effectiveTextSize.x) * m_downScale;
			}
		} else {
			float width = ei::max(m_iconSize.x, effectiveTextSize.x) + m_iconPadding * 2.0f;
			float height = m_iconSize.y + effectiveTextSize.y + m_iconPadding * 3.0f;
			// autoscale if width or height is greater then the reference frame.
			m_downScale = ei::min(1.0f, m_refFrame.width() / width,
				m_refFrame.height() / height);
			m_iconPos.x = m_textPos.x = (m_refFrame.left() + m_refFrame.right()) * 0.5f;
			m_textPos.x -= effectiveTextSize.x * m_downScale * 0.5f;
			m_iconPos.x -= m_iconSize.x * m_downScale * 0.5f;
			if(m_iconPlacement == SIDE::BOTTOM)
			{
				m_iconPos.y = m_refFrame.bottom() + m_iconPadding * m_downScale;
				m_textPos.y = m_refFrame.bottom() + (m_iconPadding * 2.0f + m_iconSize.y) * m_downScale;
			} else {
				m_iconPos.y = m_refFrame.top() - (m_iconPadding + m_iconSize.y) * m_downScale;
				m_textPos.y = m_refFrame.top() - (m_iconPadding * 2.0f + m_iconSize.y + effectiveTextSize.y) * m_downScale;
			}
		}
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

		// Icon
		if(m_iconTexture)
		{
			RefFrame rect;
			rect.sides[SIDE::LEFT]   = floorf(m_iconPos.x);
			rect.sides[SIDE::RIGHT]  = floorf(m_iconPos.x + m_iconSize.x * (isIconPlacementAbsolute() ? 1.0f : m_downScale));
			rect.sides[SIDE::BOTTOM] = floorf(m_iconPos.y);
			rect.sides[SIDE::TOP]    = floorf(m_iconPos.y + m_iconSize.y * (isIconPlacementAbsolute() ? 1.0f : m_downScale));
			GUIManager::theme().drawImage(rect, m_iconTexture);
		}

		// Text
		GUIManager::theme().drawText(m_textPos, m_text.c_str(), m_downScale * m_relativeTextSize, mouseOver);
	}

}} // namespace ca::gui