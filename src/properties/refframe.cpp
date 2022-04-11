#include "ca/gui/properties/refframe.hpp"

namespace ca { namespace gui {

	uint32 IAnchorable::s_globalGeomVersion;	///< A continiously increased number to manage the update process.

	RefFrame::RefFrame(float _l, float _r, float _b, float _t) :
		m_anchor{{this}, {this}, {this}, {this}}
	{
		m_sides[SIDE::LEFT] = _l;
		m_sides[SIDE::RIGHT] = ei::max(_l+1, _r);
		m_sides[SIDE::BOTTOM] = _b;
		m_sides[SIDE::TOP] = ei::max(_b+1, _t);
	}

	bool RefFrame::isMouseOver(const Coord2& _mousePos) const
	{
		// Simple rectangle test if the coordinate is inside
		return _mousePos.x >= left() && _mousePos.x <= right()
			&& _mousePos.y >= bottom() && _mousePos.y <= top();
	}

	Coord RefFrame::getPosition(int _dimension, float _relativePos) const
	{
		if(_dimension == 0)
			return left() + _relativePos * (right() - left());
		else
			return bottom() + _relativePos * (top() - bottom());
	}

	float RefFrame::getRelativePosition(int _dimension, Coord _position) const
	{
		if(_dimension == 0)
			return (_position - left()) / (right() - left());
		else
			return (_position - bottom()) / (top() - bottom());
	}

	void RefFrame::onExtentChanged()
	{
		IAnchorable::onExtentChanged(); // First set the new version of this object.
		IAnchorProvider::onExtentChanged(); // Then trigger updates of others
	}

	bool RefFrame::operator != (const RefFrame& _rhs) const
	{
		return m_sides[0] != _rhs.m_sides[0]
			|| m_sides[1] != _rhs.m_sides[1]
			|| m_sides[2] != _rhs.m_sides[2]
			|| m_sides[3] != _rhs.m_sides[3];
	}

	bool RefFrame::operator == (const RefFrame& _rhs) const
	{
		return m_sides[0] == _rhs.m_sides[0]
			&& m_sides[1] == _rhs.m_sides[1]
			&& m_sides[2] == _rhs.m_sides[2]
			&& m_sides[3] == _rhs.m_sides[3];
	}

	bool RefFrame::setFrame(const float _l, const float _b, const float _r, const float _t)
	{
		// Make sure the maximum coordinate never gets smaller than the minimum
		const float r = ei::max(_l+1,_r);
		const float t = ei::max(_b+1,_t);
		const bool change = (m_sides[SIDE::LEFT] != _l) || (m_sides[SIDE::RIGHT] != r)
			|| (m_sides[SIDE::BOTTOM] != _b) || (m_sides[SIDE::TOP] != t);
		if( change )
		{
			m_sides[SIDE::LEFT] = _l;
			m_sides[SIDE::BOTTOM] = _b;
			m_sides[SIDE::RIGHT] = r;
			m_sides[SIDE::TOP] = t;
			onExtentChanged();
		}
		return change;
	}


	void RefFrame::silentSetFrame(const float _l, const float _b, const float _r, const float _t)
	{
		m_sides[SIDE::LEFT] = _l;
		m_sides[SIDE::BOTTOM] = _b;
		m_sides[SIDE::RIGHT] = ei::max(_l+1,_r);
		m_sides[SIDE::TOP] = ei::max(_b+1,_t);
	}


	void RefFrame::setAnchors(const IAnchorProvider* _target, Coord _leftTarget, Coord _bottomTarget, Coord _rightTarget, Coord _topTarget)
	{
		// First detach from previous chain to keep lists of others cleen
		for(int i = 0; i < 4; ++i)
		{
			const float targetPoint = i == 0 ? _leftTarget : (i == 1 ? _bottomTarget : (i == 2 ? _rightTarget : _topTarget));
			if(targetPoint != ANCHOR::IGNORE)
				m_anchor[i].detach();
		}

		// Now construct the new one
		if(_target)
		{
			for(int i = 0; i < 4; ++i)
			{
				// Get the two horizontal or two vertical indices
				const int dim = i & 1;
				// Determine relative position of the target point towards the target frame
				const float targetPoint = i == 0 ? _leftTarget : (i == 1 ? _bottomTarget : (i == 2 ? _rightTarget : _topTarget));
				if(targetPoint != ANCHOR::IGNORE && targetPoint != ANCHOR::CLEAR)
				{
					m_anchor[i].relativePosition = _target->getRelativePosition(dim, targetPoint);
					// Determine the absolute offset between current frame boundary and target point.
					m_anchor[i].absoluteDistance = side(i) - targetPoint;
					_target->linkAnchor(m_anchor[i]);
				}
			}
		}
	}


	void RefFrame::setAnchors(const RefFrame* _targetFrame, AutoAnchorMode _mode, ANCHOR::Val _mask)
	{
		switch(_mode)
		{
		case AutoAnchorMode::ABSOLUTE:
			setAnchors(_targetFrame,
				(_mask & ANCHOR::LEFT) ? _targetFrame->left() : ANCHOR::IGNORE,
				(_mask & ANCHOR::BOTTOM) ? _targetFrame->bottom() : ANCHOR::IGNORE,
				(_mask & ANCHOR::RIGHT) ? _targetFrame->right() : ANCHOR::IGNORE,
				(_mask & ANCHOR::TOP) ? _targetFrame->top() : ANCHOR::IGNORE
			);
			break;
		case AutoAnchorMode::RELATIVE:
			setAnchors(_targetFrame,
				(_mask & ANCHOR::LEFT) ? left() : ANCHOR::IGNORE,
				(_mask & ANCHOR::BOTTOM) ? bottom() : ANCHOR::IGNORE,
				(_mask & ANCHOR::RIGHT) ? right() : ANCHOR::IGNORE,
				(_mask & ANCHOR::TOP) ? top() : ANCHOR::IGNORE
			);
			break;
		case AutoAnchorMode::SRC_CENTER:
			setAnchors(_targetFrame,
				(_mask & ANCHOR::LEFT) ? horizontalCenter() : ANCHOR::IGNORE,
				(_mask & ANCHOR::BOTTOM) ? verticalCenter() : ANCHOR::IGNORE,
				(_mask & ANCHOR::RIGHT) ? ANCHOR::CLEAR : ANCHOR::IGNORE,
				(_mask & ANCHOR::TOP) ? ANCHOR::CLEAR : ANCHOR::IGNORE
			);
			break;
		case AutoAnchorMode::DST_CENTER:
			setAnchors(_targetFrame,
				(_mask & ANCHOR::LEFT) ? _targetFrame->horizontalCenter() : ANCHOR::IGNORE,
				(_mask & ANCHOR::BOTTOM) ? _targetFrame->verticalCenter() : ANCHOR::IGNORE,
				(_mask & ANCHOR::RIGHT) ? ANCHOR::CLEAR : ANCHOR::IGNORE,
				(_mask & ANCHOR::TOP) ? ANCHOR::CLEAR : ANCHOR::IGNORE
			);
			break;
		}
	}


	void RefFrame::resetAnchors()
	{
		for(int i = 0; i < 4; ++i)
		{
			if(m_anchor[i].reference())
			{
				const int dim = (i&1);
				if(m_anchor[i].relativePosition == 0.0f || m_anchor[i].relativePosition == 1.0f)
				{
					// Interpret as absolute only positioning -> recompute offset
					m_anchor[i].absoluteDistance = side(i) - m_anchor[i].reference()->getPosition(dim, m_anchor[i].relativePosition);
				} else {
					// Recompute relative placement (keep absolute one).
					m_anchor[i].relativePosition = m_anchor[i].reference()->getRelativePosition(dim, side(i)-m_anchor[i].absoluteDistance);
				}
				// Count anchors as up to date.
				m_geomVersion = IAnchorable::getGlobalGeomVersion();
			}
		}
	}


	void RefFrame::refitToAnchors()
	{
		if(!isAnchoringEnabled()) return;
		// Check for update cycles.
		if(getGeomVersion() == IAnchorable::getGlobalGeomVersion()) return;
		// Early out if no anchor is set.
		bool hasReference = false;
		for(int i = 0; i < 4; ++i)
			hasReference |= m_anchor[i].reference() != nullptr;
		if(!hasReference) return;

		float newFrame[4];

		// Fit horizontal and vertical independent in two passes
		for(int i = 0; i < 2; ++i)
		{
			SIDE::Val sa = SIDE::Val(i);
			SIDE::Val sb = SIDE::Val(i + 2);
			if(m_anchor[sa].reference() || m_anchor[sb].reference())
			{
				if(m_anchor[sa].reference() && m_anchor[sb].reference())
				{
					// Both borders are set so resize is necessary
					newFrame[sa] = m_anchor[sa].getPosition(sa);
					newFrame[sb] = m_anchor[sb].getPosition(sa);
				} else if(m_anchor[sa].reference()) {
					float delta = m_anchor[sa].getPosition(sa) - side(sa);
					newFrame[sa] = side(sa) + delta;
					newFrame[sb] = side(sb) + delta;
				} else if(m_anchor[sb].reference()) {
					float delta = m_anchor[sb].getPosition(sa) - side(sb);
					newFrame[sa] = side(sa) + delta;
					newFrame[sb] = side(sb) + delta;
				} else {
					// Keep old
					newFrame[sa] = side(sa);
					newFrame[sb] = side(sb);
				}
			}
		}

		silentSetFrame(newFrame[0], newFrame[1], newFrame[2], newFrame[3]);
		m_geomVersion = IAnchorable::getGlobalGeomVersion();
	}

}} // namespace ca::gui