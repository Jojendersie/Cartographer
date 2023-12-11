#include "ca/gui/properties/refframe.hpp"

namespace ca { namespace gui {

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
		IAnchorProvider::onExtentChanged(); // Trigger updates of others
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
			resetAnchors();
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


	void RefFrame::setAutoAnchors(const RefFrame* _targetFrame, ANCHOR::Val _mask)
	{
		// Detach case first
		if (!_targetFrame)
		{
			if (_mask & ANCHOR::LEFT) m_anchor[SIDE::LEFT].detach();
			if (_mask & ANCHOR::BOTTOM) m_anchor[SIDE::BOTTOM].detach();
			if (_mask & ANCHOR::RIGHT) m_anchor[SIDE::RIGHT].detach();
			if (_mask & ANCHOR::TOP) m_anchor[SIDE::TOP].detach();
			return;
		}

		// Go find the closest side for each of the selected sides
		if (_mask & ANCHOR::LEFT)
		{
			const float d0 = ei::abs(left() - _targetFrame->left());
			const float d1 = ei::abs(left() - _targetFrame->right());
			if (d0 <= d1)
				m_anchor[SIDE::LEFT].attach(_targetFrame, _targetFrame->left(), left(), 0);
			else
				m_anchor[SIDE::LEFT].attach(_targetFrame, _targetFrame->right(), left(), 0);
		}
		if (_mask & ANCHOR::BOTTOM)
		{
			const float d0 = ei::abs(bottom() - _targetFrame->bottom());
			const float d1 = ei::abs(bottom() - _targetFrame->top());
			if (d0 <= d1)
				m_anchor[SIDE::BOTTOM].attach(_targetFrame, _targetFrame->bottom(), bottom(), 1);
			else
				m_anchor[SIDE::BOTTOM].attach(_targetFrame, _targetFrame->top(), bottom(), 1);
		}
		if (_mask & ANCHOR::RIGHT)
		{
			const float d0 = ei::abs(right() - _targetFrame->left());
			const float d1 = ei::abs(right() - _targetFrame->right());
			if (d0 <= d1)
				m_anchor[SIDE::RIGHT].attach(_targetFrame, _targetFrame->left(), right(), 0);
			else
				m_anchor[SIDE::RIGHT].attach(_targetFrame, _targetFrame->right(), right(), 0);
		}
		if (_mask & ANCHOR::TOP)
		{
			const float d0 = ei::abs(top() - _targetFrame->bottom());
			const float d1 = ei::abs(top() - _targetFrame->top());
			if (d0 <= d1)
				m_anchor[SIDE::TOP].attach(_targetFrame, _targetFrame->bottom(), top(), 1);
			else
				m_anchor[SIDE::TOP].attach(_targetFrame, _targetFrame->top(), top(), 1);
		}
	}


	RefFrame::AnchorPoint RefFrame::getAnchorPoint(float _relativePosX, float _relativePosY) const
	{
		return AnchorPoint {
			this,
			{ getPosition(0, _relativePosX), getPosition(1, _relativePosY) }
		};
	}


	RefFrame::AnchorPoint RefFrame::getAnchorPoint(const Coord2& _coord) const
	{
		return AnchorPoint {
			this,
			_coord
		};
	}


	void RefFrame::setAnchors(ANCHOR::Val _mask, const AnchorPoint& _anchorPoint)
	{
		if (_mask & ANCHOR::LEFT)
			m_anchor[SIDE::LEFT].attach(_anchorPoint.target, _anchorPoint.position.x, side(SIDE::LEFT), 0);
		if (_mask & ANCHOR::RIGHT)
			m_anchor[SIDE::RIGHT].attach(_anchorPoint.target, _anchorPoint.position.x, side(SIDE::RIGHT), 0);
		if (_mask & ANCHOR::BOTTOM)
			m_anchor[SIDE::BOTTOM].attach(_anchorPoint.target, _anchorPoint.position.y, side(SIDE::BOTTOM), 1);
		if (_mask & ANCHOR::TOP)
			m_anchor[SIDE::TOP].attach(_anchorPoint.target, _anchorPoint.position.y, side(SIDE::TOP), 1);
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
			}
		}
	}


	void RefFrame::refitToAnchors()
	{
		// Check for update cycles and for general anchoring ability.
		if(!IAnchorable::startRefit()) return;
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

		const bool change = (m_sides[SIDE::LEFT] != newFrame[SIDE::LEFT]) || (m_sides[SIDE::RIGHT] != newFrame[SIDE::RIGHT])
			|| (m_sides[SIDE::BOTTOM] != newFrame[SIDE::BOTTOM]) || (m_sides[SIDE::TOP] != newFrame[SIDE::TOP]);
		if (change)
		{
			// Uses silent set to avoid unnecessary resetAnchors() call
			silentSetFrame(newFrame[0], newFrame[1], newFrame[2], newFrame[3]);
			onExtentChanged();
		}
		IAnchorable::endRefit();
	}

}} // namespace ca::gui