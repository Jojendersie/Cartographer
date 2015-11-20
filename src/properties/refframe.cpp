#pragma once

#include "properties/refframe.hpp"

namespace cag {

	RefFrame::RefFrame(float _l, float _r, float _b, float _t)
	{
		sides[0] = _l;
		sides[1] = _r;
		sides[2] = _b;
		sides[3] = _t;
	}

	bool RefFrame::isMouseOver(const Coord2& _mousePos)
	{
		// Simple rectangle test if the coordinate is inside
		return _mousePos.x >= left() && _mousePos.x <= right()
			&& _mousePos.y >= bottom() && _mousePos.y <= top();
	}

} // namespace cag