#include <ca/pa/log.hpp>
#include "ca/gui/properties/anchorable.hpp"
#include "ca/gui/properties/anchorprovider.hpp"

namespace ca { namespace gui {

	Anchor::~Anchor()
	{
		// Remove this element from the double linked list
		if(prev) prev->next = next;
		if(next) next->prev = prev;
	}

	void Anchor::detach()
	{
		// Remove this element from the double linked list
		if(prev) prev->next = next;
		if(next) next->prev = prev;
		reference = nullptr;
		next = nullptr;
	}

	float Anchor::getPosition(int _a, int _b) const
	{
		if(!reference) return absoluteDistance;
		const float relRef = reference->side(_a) + relativePosition * (reference->side(_b) - reference->side(_a));
		return relRef + absoluteDistance;
	}


	
	AnchorFrame::~AnchorFrame()
	{
		// Recursively null all the linked anchors
		Anchor* current = m_anchorListStart.next;
		while(current)
		{
			Anchor* next = current->next;
			current->next = nullptr;
			current->prev = nullptr;
			current = next;
		}
	}

	void AnchorFrame::linkAnchor(Anchor& _anchor) const
	{
		_anchor.next = m_anchorListStart.next;
		_anchor.prev = &m_anchorListStart;
		_anchor.reference = this;
		if(_anchor.next)
			_anchor.next->prev = &_anchor;
		m_anchorListStart.next = &_anchor;
	}


	void AnchorFrame::onExtentChanged(const CHANGE_FLAGS::Val _changes)
	{
		if(_changes == 0) return;
		Anchor* next = m_anchorListStart.next;
		while(next)
		{
			next->self->refitToAnchors();
			next = next->next;
		}
	}



	Anchorable::Anchorable(RefFrame* _selfFrame) :
		m_selfFrame(_selfFrame),
		m_anchoringEnabled(true)
	{
		for(int i = 0; i < 4; ++i)
			m_anchor[i].self = this;
	}

	/*void Anchorable::setAnchor(AnchorFrame* _targetFrame, SIDE::Val _targetSide, SIDE::Val _sourceSide)
	{
		// First detach from previous chain to keep lists of others cleen
		m_anchor[_sourceSide].detach();
		// Now construct the new one
		if(_targetFrame)
		{
			if((_targetSide & 1) != (_sourceSide & 1))
			{
				pa::logWarning("[ca::gui] setAnchor() Target and source must be both horizontal or both vertical.");
				return;
			}
			m_anchor[_sourceSide].relativePosition = (_targetSide > 1) ? 1.0f : 0.0f;
			m_anchor[_sourceSide].absoluteDistance = m_selfFrame->sides[_sourceSide] - _targetFrame->getRefFrame().sides[_targetSide];
			_targetFrame->linkAnchor(m_anchor[_sourceSide]);
		}
	}


	void Anchorable::setAnchor(AnchorFrame* _targetFrame, SIDE::Val _sourceSide, Coord _offset)
	{
		// First detach from previous chain to keep lists of others cleen
		m_anchor[_sourceSide].detach();
		// Now construct the new one
		if(_targetFrame)
		{
			// Get the two horizontal or two vertical indices
			const int a = _sourceSide & 1;
			const int b = a + 2;
			m_anchor[_sourceSide].relativePosition = (m_selfFrame->sides[_sourceSide]-_offset - _targetFrame->getRefFrame().sides[a])
				/ (_targetFrame->getRefFrame().sides[b] - _targetFrame->getRefFrame().sides[a]);
			m_anchor[_sourceSide].absoluteDistance = _offset;
			_targetFrame->linkAnchor(m_anchor[_sourceSide]);
		}
	}*/


	void Anchorable::setAnchors(const AnchorFrame* _targetFrame, Coord _leftTarget, Coord _bottomTarget, Coord _rightTarget, Coord _topTarget)
	{
		// First detach from previous chain to keep lists of others cleen
		for(int i = 0; i < 4; ++i)
		{
			const float targetPoint = i == 0 ? _leftTarget : (i == 1 ? _bottomTarget : (i == 2 ? _rightTarget : _topTarget));
			if(targetPoint != IGNORE_ANCHOR)
				m_anchor[i].detach();
		}

		// Now construct the new one
		if(_targetFrame)
		{
			for(int i = 0; i < 4; ++i)
			{
				// Get the two horizontal or two vertical indices
				const int a = i & 1;
				const int b = a + 2;
				// Determine relative position of the target point towards the target frame
				const float targetPoint = i == 0 ? _leftTarget : (i == 1 ? _bottomTarget : (i == 2 ? _rightTarget : _topTarget));
				if(targetPoint != IGNORE_ANCHOR && targetPoint != CLEAR_ANCHOR)
				{
					const float frameSize = _targetFrame->side(b) - _targetFrame->side(a);
					m_anchor[i].relativePosition = frameSize <= 0.0f ? 0.0f
						: (targetPoint - _targetFrame->side(a)) / frameSize;
					// Determine the absolute offset between current frame boundary and target point.
					m_anchor[i].absoluteDistance = m_selfFrame->side(i) - targetPoint;
					_targetFrame->linkAnchor(m_anchor[i]);
				}
			}
		}
	}


	void Anchorable::setAnchors(const AnchorFrame* _targetFrame, AutoAnchorMode _mode, ANCHOR::Val _mask)
	{
		switch(_mode)
		{
		case AutoAnchorMode::ABSOLUTE:
			setAnchors(_targetFrame,
				(_mask & ANCHOR::LEFT) ? _targetFrame->left() : IGNORE_ANCHOR,
				(_mask & ANCHOR::BOTTOM) ? _targetFrame->bottom() : IGNORE_ANCHOR,
				(_mask & ANCHOR::RIGHT) ? _targetFrame->right() : IGNORE_ANCHOR,
				(_mask & ANCHOR::TOP) ? _targetFrame->top() : IGNORE_ANCHOR
			);
			break;
		case AutoAnchorMode::RELATIVE:
			setAnchors(_targetFrame,
				(_mask & ANCHOR::LEFT) ? m_selfFrame->left() : IGNORE_ANCHOR,
				(_mask & ANCHOR::BOTTOM) ? m_selfFrame->bottom() : IGNORE_ANCHOR,
				(_mask & ANCHOR::RIGHT) ? m_selfFrame->right() : IGNORE_ANCHOR,
				(_mask & ANCHOR::TOP) ? m_selfFrame->top() : IGNORE_ANCHOR
			);
			break;
		case AutoAnchorMode::SRC_CENTER:
			setAnchors(_targetFrame,
				(_mask & ANCHOR::LEFT) ? m_selfFrame->horizontalCenter() : IGNORE_ANCHOR,
				(_mask & ANCHOR::BOTTOM) ? m_selfFrame->verticalCenter() : IGNORE_ANCHOR,
				(_mask & ANCHOR::RIGHT) ? CLEAR_ANCHOR : IGNORE_ANCHOR,
				(_mask & ANCHOR::TOP) ? CLEAR_ANCHOR : IGNORE_ANCHOR
			);
			break;
		case AutoAnchorMode::DST_CENTER:
			setAnchors(_targetFrame,
				(_mask & ANCHOR::LEFT) ? _targetFrame->horizontalCenter() : IGNORE_ANCHOR,
				(_mask & ANCHOR::BOTTOM) ? _targetFrame->verticalCenter() : IGNORE_ANCHOR,
				(_mask & ANCHOR::RIGHT) ? CLEAR_ANCHOR : IGNORE_ANCHOR,
				(_mask & ANCHOR::TOP) ? CLEAR_ANCHOR : IGNORE_ANCHOR
			);
			break;
		}
	}


	void Anchorable::resetAnchors()
	{
		for(int i = 0; i < 4; ++i)
		{
			if(m_anchor[i].reference)
			{
				const int a = (i&1);
				const int b = 2 + (i&1);
				if(m_anchor[i].relativePosition == 0.0f || m_anchor[i].relativePosition == 1.0f)
				{
					// Interpret as absolute only positioning -> recompute offset
					m_anchor[i].absoluteDistance = m_selfFrame->side(i) 
						- (m_anchor[i].relativePosition == 0.0f ? m_anchor[i].reference->side(a) : m_anchor[i].reference->side(b));
				} else {
					// Recompute relative placement (keep absolute one).
					m_anchor[i].relativePosition = (m_selfFrame->side(i)-m_anchor[i].absoluteDistance - m_anchor[i].reference->side(a))
						/ (m_anchor[i].reference->side(b) - m_anchor[i].reference->side(a));
				}
				// Count anchors as up to date.
				m_selfFrame->geomVersion = RefFrame::getGlobalGeomVersion();
			}
		}
	}


	void Anchorable::refitToAnchors()
	{
		if(!m_anchoringEnabled) return;
		// Check for update cycles.
		if(m_selfFrame->getGeomVersion() == RefFrame::getGlobalGeomVersion()) return;
		// Early out if no anchor is set.
		bool hasReference = false;
		for(int i = 0; i < 4; ++i)
			hasReference |= m_anchor[i].reference != nullptr;
		if(!hasReference) return;

		float newFrame[4];

		// Fit horizontal and vertical independent in two passes
		for(int i = 0; i < 2; ++i)
		{
			SIDE::Val sa = SIDE::Val(i);
			SIDE::Val sb = SIDE::Val(i + 2);
			if(m_anchor[sa].reference || m_anchor[sb].reference)
			{
				if(m_anchor[sa].reference && m_anchor[sb].reference)
				{
					// Both borders are set so resize is necessary
					newFrame[sa] = m_anchor[sa].getPosition(sa, sb);
					newFrame[sb] = m_anchor[sb].getPosition(sa, sb);
				} else if(m_anchor[sa].reference) {
					float delta = m_anchor[sa].getPosition(sa, sb) - m_selfFrame->side(sa);
					newFrame[sa] = m_selfFrame->side(sa) + delta;
					newFrame[sb] = m_selfFrame->side(sb) + delta;
				} else if(m_anchor[sb].reference) {
					float delta = m_anchor[sb].getPosition(sa, sb) - m_selfFrame->side(sb);
					newFrame[sa] = m_selfFrame->side(sa) + delta;
					newFrame[sb] = m_selfFrame->side(sb) + delta;
				} else {
					// Keep old
					newFrame[sa] = m_selfFrame->side(sa);
					newFrame[sb] = m_selfFrame->side(sb);
				}
			}
		}

		m_selfFrame->setFrame(newFrame[0], newFrame[1], newFrame[2], newFrame[3], RefFrame::getGlobalGeomVersion());
	}

	void Anchorable::setAnchorable(bool _enable)
	{
		if(_enable != m_anchoringEnabled)
			resetAnchors();
		m_anchoringEnabled = _enable;
	}

}} // namespace ca::gui