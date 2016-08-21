#include "widgets/label.hpp"
#include "guimanager.hpp"
#include "rendering/theme.hpp"
#include "backend/renderbackend.hpp"

namespace ca { namespace gui {

	Label::Label() :
		Widget(true, false, false, false),
		m_textSize(0.0f),
		m_textPosition(SIDE::LEFT)
	{
	}

	void Label::draw() const
	{
		float downScale = ei::min(1.0f, ei::min(
			m_refFrame.width() / m_textSize.x,
			m_refFrame.height() / m_textSize.y
			));

		Coord2 center = Coord2(m_refFrame.left() + m_refFrame.right(), m_refFrame.bottom() + m_refFrame.top()) * 0.5f;
		center -= m_textSize * 0.5f * downScale;
		Coord2 pos;
		switch(m_textPosition)
		{
			case SIDE::LEFT: pos = Coord2(m_refFrame.left(), center.y); break;
			case SIDE::RIGHT: pos = Coord2(m_refFrame.right() - m_textSize.x*downScale, center.y); break;
			case SIDE::BOTTOM: pos = Coord2(center.x, m_refFrame.bottom()); break;
			case SIDE::TOP: pos = Coord2(center.x, m_refFrame.top() - m_textSize.y*downScale); break;
			case SIDE::CENTER: pos = center; break;
		}
		GUIManager::theme().drawText(pos, m_text.c_str(), downScale, false);
	}

	void Label::setText(const char* _text)
	{
		m_text = _text;
		m_textSize = GUIManager::theme().getTextBB(Coord2(0.0f), _text, 1.0f).max;
	}

}} // namespace ca::gui
