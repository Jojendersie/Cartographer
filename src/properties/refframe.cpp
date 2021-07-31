#include "ca/gui/properties/refframe.hpp"

namespace ca { namespace gui {

	RefFrame::RefFrame(float _l, float _r, float _b, float _t)
	{
		sides[SIDE::LEFT] = _l;
		sides[SIDE::RIGHT] = _r;
		sides[SIDE::BOTTOM] = _b;
		sides[SIDE::TOP] = _t;
	}

	bool RefFrame::isMouseOver(const Coord2& _mousePos) const
	{
		// Simple rectangle test if the coordinate is inside
		return _mousePos.x >= left() && _mousePos.x <= right()
			&& _mousePos.y >= bottom() && _mousePos.y <= top();
	}

	bool RefFrame::operator != (const RefFrame& _rhs) const
	{
		return sides[0] != _rhs.sides[0]
			|| sides[1] != _rhs.sides[1]
			|| sides[2] != _rhs.sides[2]
			|| sides[3] != _rhs.sides[3];
	}

	bool RefFrame::operator == (const RefFrame& _rhs) const
	{
		return sides[0] == _rhs.sides[0]
			&& sides[1] == _rhs.sides[1]
			&& sides[2] == _rhs.sides[2]
			&& sides[3] == _rhs.sides[3];
	}

}} // namespace ca::gui