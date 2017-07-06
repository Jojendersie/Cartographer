#include "ca/gui/widgets/progressbar.hpp"
#include "ca/gui/guimanager.hpp"
#include "ca/gui/rendering/theme.hpp"
#include "ca/gui/backend/renderbackend.hpp"

namespace ca { namespace gui {

	ProgressBar::ProgressBar() :
		Widget(true, false, false, false),
		m_backgroundTexture(0),
		m_foregroundTexture(0),
		m_progress(0.0f),
		m_vertical(false),
		m_marginLeft(0.0f),
		m_marginRight(0.0f),
		m_marginBottom(0.0f),
		m_marginTop(0.0f),
		m_textSize(1.0f)
	{
	}

	void ProgressBar::draw() const
	{
		if(m_backgroundTexture)
			GUIManager::theme().drawImage(m_refFrame, m_backgroundTexture);
		if(m_foregroundTexture)
		{
			// Compute the progress rectangle.
			RefFrame rect = m_refFrame;
			if(m_vertical)
			{
				float maxPercentHeight = m_refFrame.height() - m_marginBottom - m_marginTop;
				rect.sides[SIDE::TOP] = m_refFrame.bottom() + m_marginBottom + maxPercentHeight * m_progress;
			} else {
				float maxPercentWidth = m_refFrame.width() - m_marginLeft - m_marginRight;
				rect.sides[SIDE::RIGHT] = m_refFrame.left() + m_marginLeft + maxPercentWidth * m_progress;
			}
			// Draw the foreground clipped
			GUIManager::pushClipRegion(rect);
			GUIManager::theme().drawImage(m_refFrame, m_foregroundTexture);
			GUIManager::popClipRegion();
		}

		if(!m_text.empty())
		{
			float downScale = ei::min(1.0f, ei::min(
				m_refFrame.width() / (m_textSize.x * m_relativeTextSize),
				m_refFrame.height() / (m_textSize.y * m_relativeTextSize)
			));
			downScale *= m_relativeTextSize;

			Coord2 center = m_refFrame.center();
			center -= m_textSize * 0.5f * downScale;
			GUIManager::theme().drawText(center, m_text.c_str(), downScale, false, m_textColor);
		}
	}

	void ProgressBar::setImages(const char * _backgroundTextureFile, const char * _foregroundTextureFile, bool _smooth)
	{
		m_backgroundTexture = GUIManager::renderBackend().getTexture(_backgroundTextureFile, _smooth);
		m_foregroundTexture = GUIManager::renderBackend().getTexture(_foregroundTextureFile, _smooth);
	}

	void ProgressBar::setText(const char * _text)
	{
		m_text = _text;
		m_textSize = GUIManager::theme().getTextBB(Coord2(0.0f), _text, 1.0f).max;
	}

	void ProgressBar::setMargin(Coord _left, Coord _right, Coord _bottom, Coord _top)
	{
		m_marginLeft = ei::max(0.0f, _left);
		m_marginRight = ei::max(0.0f, _right);
		m_marginBottom = ei::max(0.0f, _bottom);
		m_marginTop = ei::max(0.0f, _top);
	}

	void ProgressBar::setProgress(float _progress)
	{
		m_progress = ei::clamp(_progress, 0.0f, 1.0f);
	}

}} // namespace ca::gui