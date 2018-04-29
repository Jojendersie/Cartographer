#include <ca/pa/log.hpp>
#include "ca/gui/properties/anchorable.hpp"
#include "ca/gui/properties/anchorprovider.hpp"

namespace ca { namespace gui {

	Anchorable::Anchorable(RefFrame* _selfFrame) :
		m_selfFrame(_selfFrame),
		m_horizontalMode(NO_RESIZE),
		m_verticalMode(NO_RESIZE),
		m_anchoringEnabled(true)
	{
	}

	void Anchorable::setAnchor(SIDE::Val _side, AnchorPtr _anchorPoint)
	{
		int si = (int)_side;
		m_anchors[si].reference = _anchorPoint;
		if(_anchorPoint)
			m_anchors[si].relativePosition = m_selfFrame->sides[si] - _anchorPoint->position;
		else m_anchors[si].relativePosition = 0.0f;
	}

	void Anchorable::autoAnchor(const IAnchorProvider* _anchorProvider)
	{
		if(!_anchorProvider) {
			pa::logWarning("[ca::gui] No anchor provider given in autoAnchor().");
			return;
		}
		setAnchor(SIDE::LEFT,   _anchorProvider->findClosestAnchor(m_selfFrame->left(), IAnchorProvider::SearchDirection::LEFT));
		setAnchor(SIDE::RIGHT,  _anchorProvider->findClosestAnchor(m_selfFrame->right(), IAnchorProvider::SearchDirection::RIGHT));
		setAnchor(SIDE::BOTTOM, _anchorProvider->findClosestAnchor(m_selfFrame->bottom(), IAnchorProvider::SearchDirection::DOWN));
		setAnchor(SIDE::TOP,    _anchorProvider->findClosestAnchor(m_selfFrame->top(), IAnchorProvider::SearchDirection::UP));
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

	void Anchorable::setAnchorModes(Mode _horizontalMode, Mode _verticalMode)
	{
		m_horizontalMode = _horizontalMode;
		m_verticalMode = _verticalMode;
	}

	bool Anchorable::refitToAnchors()
	{
		if(!m_anchoringEnabled) return false;
		// If host vanished detach this element. Early out if no anchor is set.
		bool hasReference = false;
		for(int i = 0; i < 4; ++i)
		{
			m_anchors[i].checkReference();
			hasReference |= m_anchors[i].reference != nullptr;
		}
		if(!hasReference) return false;

		RefFrame oldFrame = *m_selfFrame;

		// Fit horizontal and vertical independent in two passes
		for(int i = 0; i < 2; ++i)
		{
			SIDE::Val sa = SIDE::Val(i);
			SIDE::Val sb = SIDE::Val(i + 2);
			if(m_anchors[sa].reference || m_anchors[sb].reference)
			{
				switch(i==0 ? m_horizontalMode : m_verticalMode)
				{
				case NO_RESIZE:
					if(m_anchors[sa].reference && m_anchors[sb].reference)
					{
						// Both borders are set -> center the object
						float center = (m_selfFrame->sides[sa] + m_selfFrame->sides[sb]) * 0.5f;
						float oldAAnchor = m_selfFrame->sides[sa] - m_anchors[sa].relativePosition;
						float oldBAnchor = m_selfFrame->sides[sb] - m_anchors[sb].relativePosition;
						float relativeAPosition = (center - oldAAnchor) / (oldBAnchor - oldAAnchor);
						if(abs(oldBAnchor - oldAAnchor) < 1.0f) relativeAPosition = 0.0f;
						float delta = (m_anchors[sa].reference->position + m_anchors[sa].relativePosition - m_selfFrame->sides[sa]) * (1.0f - relativeAPosition)
									+ (m_anchors[sb].reference->position + m_anchors[sb].relativePosition - m_selfFrame->sides[sb]) * relativeAPosition;
						m_selfFrame->sides[sa] += delta;
						m_selfFrame->sides[sb] += delta;
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

	void Anchorable::setAnchorable(bool _enable)
	{
		if(_enable != m_anchoringEnabled)
			resetAnchors();
		m_anchoringEnabled = _enable;
	}

	void gui::Anchorable::Anchor::checkReference()
	{
		if(reference && !reference->host)
			reference = nullptr;
	}

}} // namespace ca::gui