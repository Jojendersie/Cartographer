#pragma once

#include "properties/anchorable.hpp"

namespace ca { namespace gui {

	Anchorable::Anchorable(RefFrame* _selfFrame) :
		m_selfFrame(_selfFrame),
		m_horizontalMode(NO_RESIZE),
		m_verticalMode(NO_RESIZE)
	{
	}

	void Anchorable::setAnchor(SIDE::Val _side, std::shared_ptr<const AnchorPoint> _anchorPoint)
	{
		int si = (int)_side;
		m_anchors[si].reference = _anchorPoint;
		m_anchors[si].relativePosition = m_selfFrame->sides[si] - _anchorPoint->position;
	}

	void Anchorable::autoAnchor(const IAnchorProvider* _anchorProvider)
	{
	}

	void Anchorable::resetAnchors()
	{
		for(int i = 0; i < 4; ++i)
		{
			m_anchors[i].checkReference();
			if(m_anchors[i].reference)
				m_anchors[i].relativePosition = m_selfFrame->sides[i] - m_anchors[i].reference->position;
		}
	}

	void Anchorable::setHorizontalAnchorMode(Mode _mode)
	{
		m_horizontalMode = _mode;
	}

	void Anchorable::setVerticalAnchorMode(Mode _mode)
	{
		m_verticalMode = _mode;
	}

	void Anchorable::setAnchorModes(Mode _mode)
	{
		m_horizontalMode = _mode;
		m_verticalMode = _mode;
	}

	bool Anchorable::refitToAnchors()
	{
		// If host vanished detach this element
		for(int i = 0; i < 4; ++i)
			m_anchors[i].checkReference();

		RefFrame oldFrame = *m_selfFrame;

		// Fit horizontal and vertical independent in two passes
		for(int i = 0; i < 2; ++i)
		{
			SIDE::Val sa = SIDE::Val(i * 2);
			SIDE::Val sb = SIDE::Val(i * 2 + 1);
			if(m_anchors[sa].reference || m_anchors[sb].reference)
			{
				switch(i==0 ? m_horizontalMode : m_verticalMode)
				{
				case NO_RESIZE:
					if(m_anchors[sa].reference && m_anchors[sb].reference)
					{
						// Both borders are set -> center the object
						float center = (m_selfFrame->sides[sa] + m_selfFrame->sides[sb]) * 0.5f;
						float oldLeftAnchor = m_selfFrame->sides[sa] - m_anchors[sa].relativePosition;
						float oldRightAnchor = m_selfFrame->sides[sb] - m_anchors[sb].relativePosition;
						float relativeLeftPosition = (center - oldLeftAnchor) / (oldRightAnchor - oldLeftAnchor);
						float delta = (m_anchors[sa].reference->position + m_anchors[sa].relativePosition - m_selfFrame->sides[sa]) * (1.0f - relativeLeftPosition)
									+ (m_anchors[sb].reference->position + m_anchors[sb].relativePosition - m_selfFrame->sides[sb]) * relativeLeftPosition;
						m_selfFrame->sides[sa] += delta;
						m_selfFrame->sides[sb] += delta;

						// ?? what is this block for?
						/*float ad = m_anchors[sb].reference->position - m_anchors[sa].reference->position;
						float middle = m_anchors[sa].reference->position + 0.5f * ad;
						float width = m_selfFrame->width();
						m_selfFrame->sides[sa] = middle - width * 0.5f;
						m_selfFrame->sides[sb] = middle + width * 0.5f;*/
					} else if(m_anchors[sa].reference) {
						float delta = m_anchors[sa].reference->position + m_anchors[sa].relativePosition - m_selfFrame->sides[sa];
						m_selfFrame->sides[sa] += delta;
						m_selfFrame->sides[sb] += delta;
					} else if(m_anchors[sb].reference) {
						float delta = m_anchors[sb].reference->position + m_anchors[sb].relativePosition - m_selfFrame->sides[sb];
						m_selfFrame->sides[sa] += delta;
						m_selfFrame->sides[sb] += delta;
					}
					break;
				case PREFER_MOVE:
					if(m_anchors[sa].reference && m_anchors[sb].reference)
					{
						// Both borders are set so resize is necessary
						m_selfFrame->sides[sa] = m_anchors[sa].reference->position + m_anchors[sa].relativePosition;
						m_selfFrame->sides[sb] = m_anchors[sb].reference->position + m_anchors[sb].relativePosition;
					} else if(m_anchors[sa].reference) {
						float delta = m_anchors[sa].reference->position + m_anchors[sa].relativePosition - m_selfFrame->sides[sa];
						m_selfFrame->sides[sa] += delta;
						m_selfFrame->sides[sb] += delta;
					} else if(m_anchors[sb].reference) {
						float delta = m_anchors[sb].reference->position + m_anchors[sb].relativePosition - m_selfFrame->sides[sb];
						m_selfFrame->sides[sa] += delta;
						m_selfFrame->sides[sb] += delta;
					}
					break;
				case PREFER_RESIZE:
					if(m_anchors[sa].reference)
						m_selfFrame->sides[sa] = m_anchors[sa].reference->position + m_anchors[sa].relativePosition;
					if(m_anchors[sb].reference)
						m_selfFrame->sides[sb] = m_anchors[sb].reference->position + m_anchors[sb].relativePosition;
					break;
				}
			}
		}

		return oldFrame != *m_selfFrame;
	}

	void gui::Anchorable::Anchor::checkReference()
	{
		if(reference && !reference->host)
			reference = nullptr;
	}

}} // namespace ca::gui