#pragma once

#include "properties/anchorable.hpp"

namespace ca { namespace gui {

	Anchorable::Anchorable(RefFrame* _selfFrame) : m_selfFrame(_selfFrame)
	{
	}

	void Anchorable::setAnchor(SIDE::Val _side, std::shared_ptr<const AnchorPoint> _anchorPoint)
	{
		int si = (int)_side;
		m_anchors[si].reference = _anchorPoint;
		m_anchors[si].relativePosition = m_selfFrame->sides[si] - _anchorPoint->position[si/2];
	}

	void Anchorable::resetAnchors()
	{
		for(int i = 0; i < 4; ++i)
		{
			m_anchors[i].checkReference();
			if(m_anchors[i].reference)
				m_anchors[i].relativePosition = m_selfFrame->sides[i] - m_anchors[i].reference->position[i/2];
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

	bool Anchorable::refitToAnchors()
	{
		// If host vanished detach this element
		for(int i = 0; i < 4; ++i)
			m_anchors[i].checkReference();

		// Fit horizontal and vertical independent
		// horizontal ****************************
		if(m_anchors[SIDE::LEFT].reference || m_anchors[SIDE::RIGHT].reference)
		{
			switch(m_horizontalMode)
			{
			case NO_RESIZE:
				if(m_anchors[SIDE::LEFT].reference && m_anchors[SIDE::RIGHT].reference)
				{
					// Both borders are set -> center the object
					float ad = m_anchors[SIDE::RIGHT].reference->position.x - m_anchors[SIDE::LEFT].reference->position.x;
					float middle = m_anchors[SIDE::LEFT].reference->position.x + 0.5f * ad;
					m_selfFrame->sides[SIDE::LEFT] = middle - m_selfFrame->width() * 0.5f;
					m_selfFrame->sides[SIDE::RIGHT] = middle + m_selfFrame->width() * 0.5f;
				} else if(m_anchors[SIDE::LEFT].reference) {
					float delta = m_anchors[SIDE::LEFT].reference->position.x + m_anchors[SIDE::LEFT].relativePosition - m_selfFrame->sides[SIDE::LEFT];
					m_selfFrame->sides[SIDE::LEFT] += delta;
					m_selfFrame->sides[SIDE::RIGHT] += delta;
				} else if(m_anchors[SIDE::RIGHT].reference) {
					float delta = m_anchors[SIDE::RIGHT].reference->position.x + m_anchors[SIDE::RIGHT].relativePosition - m_selfFrame->sides[SIDE::RIGHT];
					m_selfFrame->sides[SIDE::LEFT] += delta;
					m_selfFrame->sides[SIDE::RIGHT] += delta;
				}
				break;
			case PREFER_MOVE:
				if(m_anchors[SIDE::LEFT].reference && m_anchors[SIDE::RIGHT].reference)
				{
					// Both borders are set so resize is necessary
					m_selfFrame->sides[SIDE::LEFT] = m_anchors[SIDE::LEFT].reference->position.x + m_anchors[SIDE::LEFT].relativePosition;
					m_selfFrame->sides[SIDE::RIGHT] = m_anchors[SIDE::RIGHT].reference->position.x + m_anchors[SIDE::RIGHT].relativePosition;
				} else if(m_anchors[SIDE::LEFT].reference) {
					float delta = m_anchors[SIDE::LEFT].reference->position.x + m_anchors[SIDE::LEFT].relativePosition - m_selfFrame->sides[SIDE::LEFT];
					m_selfFrame->sides[SIDE::LEFT] += delta;
					m_selfFrame->sides[SIDE::RIGHT] += delta;
				} else if(m_anchors[SIDE::RIGHT].reference) {
					float delta = m_anchors[SIDE::RIGHT].reference->position.x + m_anchors[SIDE::RIGHT].relativePosition - m_selfFrame->sides[SIDE::RIGHT];
					m_selfFrame->sides[SIDE::LEFT] += delta;
					m_selfFrame->sides[SIDE::RIGHT] += delta;
				}
				break;
			case PREFER_RESIZE:
				if(m_anchors[SIDE::LEFT].reference)
					m_selfFrame->sides[SIDE::LEFT] = m_anchors[SIDE::LEFT].reference->position.x + m_anchors[SIDE::LEFT].relativePosition;
				if(m_anchors[SIDE::RIGHT].reference)
					m_selfFrame->sides[SIDE::RIGHT] = m_anchors[SIDE::RIGHT].reference->position.x + m_anchors[SIDE::RIGHT].relativePosition;
				break;
			}
		}

		// vertical ******************************
		if(m_anchors[SIDE::BOTTOM].reference || m_anchors[SIDE::TOP].reference)
		{
			switch(m_horizontalMode)
			{
			case NO_RESIZE:
				if(m_anchors[SIDE::BOTTOM].reference && m_anchors[SIDE::TOP].reference)
				{
					// Both borders are set -> center the object
					float ad = m_anchors[SIDE::TOP].reference->position.y - m_anchors[SIDE::BOTTOM].reference->position.y;
					float middle = m_anchors[SIDE::BOTTOM].reference->position.y + 0.5f * ad;
					m_selfFrame->sides[SIDE::BOTTOM] = middle - m_selfFrame->height() * 0.5f;
					m_selfFrame->sides[SIDE::TOP] = middle + m_selfFrame->height() * 0.5f;
				} else if(m_anchors[SIDE::BOTTOM].reference) {
					float delta = m_anchors[SIDE::BOTTOM].reference->position.y + m_anchors[SIDE::BOTTOM].relativePosition - m_selfFrame->sides[SIDE::BOTTOM];
					m_selfFrame->sides[SIDE::BOTTOM] += delta;
					m_selfFrame->sides[SIDE::TOP] += delta;
				} else if(m_anchors[SIDE::TOP].reference) {
					float delta = m_anchors[SIDE::TOP].reference->position.y + m_anchors[SIDE::TOP].relativePosition - m_selfFrame->sides[SIDE::TOP];
					m_selfFrame->sides[SIDE::BOTTOM] += delta;
					m_selfFrame->sides[SIDE::TOP] += delta;
				}
				break;
			case PREFER_MOVE:
				if(m_anchors[SIDE::BOTTOM].reference && m_anchors[SIDE::TOP].reference)
				{
					// Both borders are set so resize is necessary
					m_selfFrame->sides[SIDE::BOTTOM] = m_anchors[SIDE::BOTTOM].reference->position.y + m_anchors[SIDE::BOTTOM].relativePosition;
					m_selfFrame->sides[SIDE::TOP] = m_anchors[SIDE::TOP].reference->position.y + m_anchors[SIDE::TOP].relativePosition;
				} else if(m_anchors[SIDE::BOTTOM].reference) {
					float delta = m_anchors[SIDE::BOTTOM].reference->position.y + m_anchors[SIDE::BOTTOM].relativePosition - m_selfFrame->sides[SIDE::BOTTOM];
					m_selfFrame->sides[SIDE::BOTTOM] += delta;
					m_selfFrame->sides[SIDE::TOP] += delta;
				} else if(m_anchors[SIDE::TOP].reference) {
					float delta = m_anchors[SIDE::TOP].reference->position.y + m_anchors[SIDE::TOP].relativePosition - m_selfFrame->sides[SIDE::TOP];
					m_selfFrame->sides[SIDE::BOTTOM] += delta;
					m_selfFrame->sides[SIDE::TOP] += delta;
				}
				break;
			case PREFER_RESIZE:
				if(m_anchors[SIDE::BOTTOM].reference)
					m_selfFrame->sides[SIDE::BOTTOM] = m_anchors[SIDE::BOTTOM].reference->position.y + m_anchors[SIDE::BOTTOM].relativePosition;
				if(m_anchors[SIDE::TOP].reference)
					m_selfFrame->sides[SIDE::TOP] = m_anchors[SIDE::TOP].reference->position.y + m_anchors[SIDE::TOP].relativePosition;
				break;
			}
		}
		return false;
	}

	void gui::Anchorable::Anchor::checkReference()
	{
		if(reference && !reference->host)
			reference = nullptr;
	}

}} // namespace ca::gui