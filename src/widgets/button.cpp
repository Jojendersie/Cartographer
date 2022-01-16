#include "ca/gui/widgets/button.hpp"
#include "ca/gui/guimanager.hpp"
#include "ca/gui/rendering/theme.hpp"
#include "ca/gui/backend/renderbackend.hpp"

namespace ca { namespace gui {

	Button::Button() :
		Clickable(this),
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

	void Button::setIconImg(const char * _textureFile, bool _smooth)
	{
		m_iconTexture = GUIManager::renderBackend().getTexture(_textureFile, _smooth);
	}

	void Button::setIconExtent(SIDE::Val _side, const Coord2& _size, Coord _padding)
	{
		m_iconPlacement = _side;
		m_iconSize = _size;
		m_iconPadding = _padding;
		recomputeIconNTextPositions();
	}

	void Button::setIconExtent(const Coord2 & _position, const Coord2 & _size)
	{
		m_iconPlacement = SIDE::CENTER; // Mark absolute positions with an invalid value
		m_iconSize = _size;
		m_iconPadding = 0.0f;
		m_iconPosRel = _position - m_refFrame.center();
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
		Coord2 refCenter = m_refFrame.center();
		if(!m_iconTexture || isIconPlacementAbsolute())
		{
			// autoscale if width or height is greater then the reference frame.
			m_downScale = ei::min(1.0f, m_refFrame.width() / (effectiveTextSize.x),
				m_refFrame.height() / (effectiveTextSize.y));
			m_textPos = refCenter;
			m_textPos -= effectiveTextSize * m_downScale * 0.5f;
		} else if(m_text.empty()) {
			// autoscale if width or height is greater then the reference frame.
			m_downScale = ei::min(1.0f, m_refFrame.width() / (m_iconSize.x + m_iconPadding * 2.0f),
				m_refFrame.height() / (m_iconSize.y + m_iconPadding * 2.0f));
			// Center the icon
			m_iconPosRel = -m_iconSize * m_downScale * 0.5f;
		} else if(m_iconPlacement == SIDE::LEFT || m_iconPlacement == SIDE::RIGHT)
		{
			float width = m_iconSize.x + effectiveTextSize.x + m_iconPadding * 2.0f;
			float height = ei::max(m_iconSize.y, effectiveTextSize.y) + m_iconPadding * 2.0f;
			// autoscale if width or height is greater then the reference frame.
			m_downScale = ei::min(1.0f, m_refFrame.width() / width,
				m_refFrame.height() / height);
			m_textPos.y = refCenter.y;
			m_iconPosRel.y = 0.0f;
			m_textPos.y -= effectiveTextSize.y * m_downScale * 0.5f;
			m_iconPosRel.y -= m_iconSize.y * m_downScale * 0.5f;
			if(m_iconPlacement == SIDE::LEFT)
			{
				m_iconPosRel.x = m_refFrame.left() + m_iconPadding * m_downScale - refCenter.x;
				m_textPos.x = m_refFrame.left() + (m_iconPadding * 2.0f + m_iconSize.x) * m_downScale;
			} else {
				m_iconPosRel.x = m_refFrame.right() - (m_iconPadding + m_iconSize.x) * m_downScale - refCenter.x;
				m_textPos.x = m_refFrame.right() - (m_iconPadding * 2.0f + m_iconSize.x + effectiveTextSize.x) * m_downScale;
			}
		} else {
			float width = ei::max(m_iconSize.x, effectiveTextSize.x) + m_iconPadding * 2.0f;
			float height = m_iconSize.y + effectiveTextSize.y + m_iconPadding * 3.0f;
			// autoscale if width or height is greater then the reference frame.
			m_downScale = ei::min(1.0f, m_refFrame.width() / width,
				m_refFrame.height() / height);
			m_textPos.x = refCenter.x;
			m_iconPosRel.x = 0.0f;
			m_textPos.x -= effectiveTextSize.x * m_downScale * 0.5f;
			m_iconPosRel.x -= m_iconSize.x * m_downScale * 0.5f;
			if(m_iconPlacement == SIDE::BOTTOM)
			{
				m_iconPosRel.y = m_refFrame.bottom() + m_iconPadding * m_downScale - refCenter.y;
				m_textPos.y = m_refFrame.bottom() + (m_iconPadding * 2.0f + m_iconSize.y) * m_downScale;
			} else {
				m_iconPosRel.y = m_refFrame.top() - (m_iconPadding + m_iconSize.y) * m_downScale - refCenter.y;
				m_textPos.y = m_refFrame.top() - (m_iconPadding * 2.0f + m_iconSize.y + effectiveTextSize.y) * m_downScale;
			}
		}
	}

	void Button::draw() const
	{
		// Background
		bool mouseOver = isMouseOver();
		if(m_backgroundTexture) {
			if(isAnyButtonDown() && m_downTexture)
				GUIManager::theme().drawImage(m_refFrame, m_downTexture);
			else if(mouseOver && m_hoverTexture)
				GUIManager::theme().drawImage(m_refFrame, m_hoverTexture);
			else
				GUIManager::theme().drawImage(m_refFrame, m_backgroundTexture);
		} else
			GUIManager::theme().drawButton(m_refFrame, mouseOver, isAnyButtonDown(), true);

		// Icon
		if(m_iconTexture)
		{
			GUIManager::theme().drawImage(computeIconRect(), m_iconTexture);
		}

		// Text
		GUIManager::theme().drawText(m_textPos, m_text.c_str(), m_downScale * m_relativeTextSize, mouseOver, m_textColor);
	}

	RefFrame Button::computeIconRect() const
	{
		Coord2 iconPos = m_iconPosRel + m_refFrame.center();
		RefFrame rect;
		rect.sides[SIDE::LEFT]   = floorf(iconPos.x);
		rect.sides[SIDE::RIGHT]  = floorf(iconPos.x + m_iconSize.x * (isIconPlacementAbsolute() ? 1.0f : m_downScale));
		rect.sides[SIDE::BOTTOM] = floorf(iconPos.y);
		rect.sides[SIDE::TOP]    = floorf(iconPos.y + m_iconSize.y * (isIconPlacementAbsolute() ? 1.0f : m_downScale));
		return rect;
	}

}} // namespace ca::gui