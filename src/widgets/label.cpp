#include "ca/gui/widgets/label.hpp"
#include "ca/gui/guimanager.hpp"
#include "ca/gui/rendering/theme.hpp"
#include "ca/gui/backend/renderbackend.hpp"

namespace ca { namespace gui {

	Label::Label() :
		m_textSize(0.0f),
		m_margin(0.0f),
		m_textPosition(SIDE::LEFT),
		m_backgroundColor(-1.0f)
	{
		setEnabled(false);
	}

	void Label::draw() const
	{
		if(m_backgroundColor.a > 0.0f)
			GUIManager::theme().drawBackgroundArea(m_refFrame, m_backgroundColor.a, ei::Vec3(m_backgroundColor));

		float downScale = ei::min(1.0f, ei::min(
			m_refFrame.width() / (m_textSize.x * m_relativeTextSize + 2.0f * m_margin),
			m_refFrame.height() / (m_textSize.y * m_relativeTextSize + 2.0f * m_margin)
			));
		downScale *= m_relativeTextSize;
		float margin = downScale * m_margin;

		Coord2 center = m_refFrame.center();
		center -= m_textSize * 0.5f * downScale;
		Coord2 pos;
		switch(m_textPosition)
		{
			case SIDE::LEFT: pos = Coord2(m_refFrame.left() + margin, center.y); break;
			case SIDE::RIGHT: pos = Coord2(m_refFrame.right() - m_textSize.x*downScale - margin, center.y); break;
			case SIDE::BOTTOM: pos = Coord2(center.x, m_refFrame.bottom() + margin); break;
			case SIDE::TOP: pos = Coord2(center.x, m_refFrame.top() - m_textSize.y*downScale - margin); break;
			case SIDE::CENTER: pos = center; break;
		}
		GUIManager::theme().drawText(pos, m_text.c_str(), downScale, false, m_textColor);
	}

	void Label::setText(const char* _text, float _fitSize)
	{
		m_text = _text;
		m_textSize = GUIManager::theme().getTextBB(Coord2(0.0f), _text, 1.0f).max;
		// Automatically change the label size
		if(_fitSize >= 0.0f) {
			m_refFrame.rect.max = m_refFrame.rect.min + m_textSize + _fitSize;
			m_refFrame.rect.min -= _fitSize;
			m_margin = _fitSize;
		}
	}

}} // namespace ca::gui
