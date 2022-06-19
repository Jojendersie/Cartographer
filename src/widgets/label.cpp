#include "ca/gui/widgets/label.hpp"
#include "ca/gui/guimanager.hpp"
#include "ca/gui/rendering/theme.hpp"
#include "ca/gui/backend/renderbackend.hpp"

namespace ca { namespace gui {

	Label::Label() :
		m_textSize(0.0f),
		m_margin(0.0f, 0.0f),
		m_textPosition(SIDE::LEFT),
		m_backgroundColor(-1.0f)
	{
		setEnabled(false);
	}

	Label::Label(const char* _text, float _fitSize, float _opacity) :
		Label()
	{
		setText(_text, _fitSize);
		setBackgroundOpacity(_opacity);
	}

	void Label::draw() const
	{
		if(m_backgroundColor.a > 0.0f)
			GUIManager::theme().drawBackgroundArea(rectangle(), m_backgroundColor.a, ei::Vec3(m_backgroundColor));

		float downScale = ei::min(1.0f, ei::min(
			width() / (m_textSize.x * m_relativeTextSize + 2.0f * m_margin.x),
			height() / (m_textSize.y * m_relativeTextSize + 2.0f * m_margin.y)
			));
		downScale *= m_relativeTextSize;
		Coord2 margin = downScale * m_margin;

		Coord2 center = this->center();
		center -= m_textSize * 0.5f * downScale;
		Coord2 pos;
		switch(m_textPosition)
		{
			case SIDE::LEFT: pos = Coord2(left() + margin.x, center.y); break;
			case SIDE::RIGHT: pos = Coord2(right() - m_textSize.x*downScale - margin.x, center.y); break;
			case SIDE::BOTTOM: pos = Coord2(center.x, bottom() + margin.y); break;
			case SIDE::TOP: pos = Coord2(center.x, top() - m_textSize.y*downScale - margin.y); break;
			case SIDE::CENTER: pos = center; break;
		}
		GUIManager::theme().drawText(pos, m_text.c_str(), downScale, false, m_textColor);
	}

	void Label::setText(const char* _text, float _fitSize)
	{
		m_text = _text;
		ei::Rect2D textDim = GUIManager::theme().getTextBB(Coord2(0.0f), _text, 1.0f);
		m_textSize = textDim.max - textDim.min;
		// Automatically change the label size
		if(_fitSize >= 0.0f) {
			setMargin(_fitSize);
			setFrame(left(), bottom(),
				left() + m_textSize.x + _fitSize * 2.0f,
				bottom() + m_textSize.y + _fitSize * 2.0f);
		}
	}

}} // namespace ca::gui
