#pragma once

#include "properties/refframe.hpp"

namespace cag {

	bool RefFrame::isMouseOver(const Coord2& _mousePos)
	{
		// Simple rectangle test if the coordinate is inside
		return _mousePos.x >= left() && _mousePos.x <= right()
			&& _mousePos.y >= bottom() && _mousePos.y <= top();
	}

} // namespace cag