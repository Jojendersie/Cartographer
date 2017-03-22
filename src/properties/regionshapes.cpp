#pragma once

#include "ca/gui/properties/regionshapes.hpp"

namespace ca { namespace gui {

	EllipseRegion::EllipseRegion(const RefFrame* _selfFrame) :
		m_selfFrame(_selfFrame)
	{
	}

	bool EllipseRegion::isMouseOver(const Coord2& _mousePos) const
	{
		//Coord2 pos = _mousePos - m_selfFrame->center();
		//pos *= 2.0f;
		// Convert _mousePos to [-1,1] positions inside the frame
		Coord2 pos = _mousePos - Coord2(m_selfFrame->left(), m_selfFrame->bottom());
		pos /= m_selfFrame->size();
		pos = pos * 2.0f - 1.0f;
		// Check if the position is inside a circle
		return lensq(pos) <= 1.0f;
	}

}} // namespace ca::gui
