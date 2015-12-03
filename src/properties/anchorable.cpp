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
					float center = (m_selfFrame->sides[SIDE::LEFT] + m_selfFrame->sides[SIDE::RIGHT]) * 0.5f;
					float oldLeftAnchor = m_selfFrame->sides[SIDE::LEFT] - m_anchors[SIDE::LEFT].relativePosition;
					float oldRightAnchor = m_selfFrame->sides[SIDE::RIGHT] - m_anchors[SIDE::RIGHT].relativePosition;
					float relativeLeftPosition = (center - oldLeftAnchor) / (oldRightAnchor - oldLeftAnchor);
					float delta = (m_anchors[SIDE::LEFT].reference->position.y + m_anchors[SIDE::LEFT].relativePosition - m_selfFrame->sides[SIDE::LEFT]) * (1.0f - relativeLeftPosition)
								+ (m_anchors[SIDE::RIGHT].reference->position.y + m_anchors[SIDE::RIGHT].relativePosition - m_selfFrame->sides[SIDE::RIGHT]) * relativeLeftPosition;
					m_selfFrame->sides[SIDE::LEFT] += delta;
					m_selfFrame->sides[SIDE::RIGHT] += delta;

					float ad = m_anchors[SIDE::RIGHT].reference->position.x - m_anchors[SIDE::LEFT].reference->position.x;
					float middle = m_anchors[SIDE::LEFT].reference->position.x + 0.5f * ad;
					float width = m_selfFrame->width();
					m_selfFrame->sides[SIDE::LEFT] = middle - width * 0.5f;
					m_selfFrame->sides[SIDE::RIGHT] = middle + width * 0.5f;
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
			switch(m_verticalMode)
			{
			case NO_RESIZE:
				if(m_anchors[SIDE::BOTTOM].reference && m_anchors[SIDE::TOP].reference)
				{
					// Both borders are set -> center the object
					float center = (m_selfFrame->sides[SIDE::BOTTOM] + m_selfFrame->sides[SIDE::TOP]) * 0.5f;
					float oldBottomAnchor = m_selfFrame->sides[SIDE::BOTTOM] - m_anchors[SIDE::BOTTOM].relativePosition;
					float oldTopAnchor = m_selfFrame->sides[SIDE::TOP] - m_anchors[SIDE::TOP].relativePosition;
					float relativeBottomPosition = (center - oldBottomAnchor) / (oldTopAnchor - oldBottomAnchor);
					float delta = (m_anchors[SIDE::BOTTOM].reference->position.y + m_anchors[SIDE::BOTTOM].relativePosition - m_selfFrame->sides[SIDE::BOTTOM]) * (1.0f - relativeBottomPosition)
								+ (m_anchors[SIDE::TOP].reference->position.y + m_anchors[SIDE::TOP].relativePosition - m_selfFrame->sides[SIDE::TOP]) * relativeBottomPosition;
					m_selfFrame->sides[SIDE::BOTTOM] += delta;
					m_selfFrame->sides[SIDE::TOP] += delta;
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
					//goto Resize_Top_Down;
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
				// Relative resize implementation: destroys relative margins/anchors
				/*if(m_anchors[SIDE::BOTTOM].reference && m_anchors[SIDE::TOP].reference)
				{
Resize_Top_Down:
					float oldBottomAnchor = m_selfFrame->sides[SIDE::BOTTOM] - m_anchors[SIDE::BOTTOM].relativePosition;
					float oldTopAnchor = m_selfFrame->sides[SIDE::TOP] - m_anchors[SIDE::TOP].relativePosition;
					float oldSize = oldTopAnchor - oldBottomAnchor;
					float newBottomAnchor = m_anchors[SIDE::BOTTOM].reference->position.y;
					float newTopAnchor = m_anchors[SIDE::TOP].reference->position.y;
					float newSize = newTopAnchor - newBottomAnchor;
					float relativeBottomPosition = m_anchors[SIDE::BOTTOM].relativePosition / oldSize;
					float relativeTopPosition = m_anchors[SIDE::TOP].relativePosition / oldSize;
					m_anchors[SIDE::BOTTOM].relativePosition = relativeBottomPosition * newSize;
					m_anchors[SIDE::TOP].relativePosition = relativeTopPosition * newSize;
					m_selfFrame->sides[SIDE::BOTTOM] = relativeBottomPosition * newSize + newBottomAnchor;
					m_selfFrame->sides[SIDE::TOP] = relativeTopPosition * newSize + newTopAnchor;
				} else */
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