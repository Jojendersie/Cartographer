#include "ca/gui/properties/refframe.hpp"

namespace ca { namespace gui {

	uint32 RefFrame::geomVersion;	///< A continiously increased number to manage the update process.

	RefFrame::RefFrame(float _l, float _r, float _b, float _t)
	{
		m_sides[SIDE::LEFT] = _l;
		m_sides[SIDE::RIGHT] = ei::max(_l+1, _r);
		m_sides[SIDE::BOTTOM] = _b;
		m_sides[SIDE::TOP] = ei::max(_b+1, _t);
		m_geomVersion = getGlobalGeomVersion();
	}

	bool RefFrame::isMouseOver(const Coord2& _mousePos) const
	{
		// Simple rectangle test if the coordinate is inside
		return _mousePos.x >= left() && _mousePos.x <= right()
			&& _mousePos.y >= bottom() && _mousePos.y <= top();
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

	CHANGE_FLAGS::Val RefFrame::setFrame(const float _l, const float _b, const float _r, const float _t)
	{
				// Make sure the maximum coordinate never gets smaller than the minimum
		const float r = ei::max(_l+1,_r);
		const float t = ei::max(_b+1,_t);
		CHANGE_FLAGS::Val flags = ((m_sides[SIDE::LEFT] != _l) ? CHANGE_FLAGS::LEFT : 0)
			| ((m_sides[SIDE::RIGHT] != _r) ? CHANGE_FLAGS::RIGHT : 0)
			| ((m_sides[SIDE::BOTTOM] != r) ? CHANGE_FLAGS::BOTTOM : 0)
			| ((m_sides[SIDE::TOP] != t) ? CHANGE_FLAGS::TOP : 0);
		if( flags )
		{
			m_sides[SIDE::LEFT] = _l;
			m_sides[SIDE::BOTTOM] = _b;
			m_sides[SIDE::RIGHT] = r;
			m_sides[SIDE::TOP] = t;
			++geomVersion;
			m_geomVersion = geomVersion;
			onExtentChanged(flags);
		}
		return flags;
	}

	void RefFrame::setFrame(const float _l, const float _b, const float _r, const float _t, const uint32 _geomVersion)
	{
		// Make sure the maximum coordinate never gets smaller than the minimum
		const float r = ei::max(_l+1,_r);
		const float t = ei::max(_b+1,_t);
		CHANGE_FLAGS::Val flags = ((left() != _l) ? CHANGE_FLAGS::LEFT : 0)
			| ((right() != r) ? CHANGE_FLAGS::RIGHT : 0)
			| ((bottom() != _b) ? CHANGE_FLAGS::BOTTOM : 0)
			| ((top() != t) ? CHANGE_FLAGS::TOP : 0);
		m_geomVersion = _geomVersion;
		if( flags )
		{
			m_sides[SIDE::LEFT] = _l;
			m_sides[SIDE::BOTTOM] = _b;
			m_sides[SIDE::RIGHT] = r;
			m_sides[SIDE::TOP] = t;
			onExtentChanged(flags);
		}
	}


	void RefFrame::silentSetFrame(const float _l, const float _b, const float _r, const float _t)
	{
		m_sides[SIDE::LEFT] = _l;
		m_sides[SIDE::BOTTOM] = _b;
		m_sides[SIDE::RIGHT] = ei::max(_l+1,_r);
		m_sides[SIDE::TOP] = ei::max(_b+1,_t);
	}

}} // namespace ca::gui