#include "ca/gui/properties/regionshapes.hpp"

using namespace ei;

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


	AnnulusSegmentRegion::AnnulusSegmentRegion(
		const Coord2& _center,
		Coord _innerRadius,
		Coord _outerRadius,
		float _startAngle,
		float _endAngle
	) :
		m_relativeCenter(_center),
		m_innerRadius(min(_innerRadius, _outerRadius)),
		m_outerRadius(max(_innerRadius, _outerRadius)),
		m_startAngle(ei::mod(_startAngle, 2.0f*ei::PI)),
		m_endAngle(ei::mod(_endAngle, 2.0f*ei::PI))
	{
		m_innerRadiusSq = m_innerRadius * m_innerRadius;
		m_outerRadiusSq = m_outerRadius * m_outerRadius;
	}

	bool AnnulusSegmentRegion::isMouseOver(const Coord2 & _mousePos) const
	{
		Coord2 toMouse = _mousePos - (Coord2(m_selfFrame->left(), m_selfFrame->bottom()) + m_relativeCenter);

		// Test if the point is within the ring
		float distSq = lensq(toMouse);
		if(distSq < m_innerRadiusSq || distSq > m_outerRadiusSq)
			return false;

		// Compute an angle and check if the segment is correct
		float angle = atan2(toMouse.y, toMouse.x);
		if(angle < 0.0f)
			angle += 2.0f * ei::PI;

		return isAngleInRange(angle);
	}

	void AnnulusSegmentRegion::attach(RefFrame & _frame)
	{
		// If the up-vector is inside the segment the maximum component is center+radius.
		// The same goes for all other extremal vectors.
		// If one of the vectors is not in the segment the maximum/minimum is determined
		// by one of the boundary points.
		if( isAngleInRange(PI/2.0f) )
			_frame.sides[SIDE::TOP] = m_relativeCenter.y + m_outerRadius;
		else {
			float t = max(sin(m_startAngle), sin(m_endAngle));
			_frame.sides[SIDE::TOP] = m_relativeCenter.y + (t < 0.0f ? m_innerRadius * t : m_outerRadius * t);
		}
		if( isAngleInRange(PI*1.5f) )
			_frame.sides[SIDE::BOTTOM] = m_relativeCenter.y - m_outerRadius;
		else {
			float t = min(sin(m_startAngle), sin(m_endAngle));
			_frame.sides[SIDE::BOTTOM] = m_relativeCenter.y + (t > 0.0f ? m_innerRadius * t : m_outerRadius * t);
		}
		if( isAngleInRange(0.0f) )
			_frame.sides[SIDE::RIGHT] = m_relativeCenter.x + m_outerRadius;
		else {
			float t = max(cos(m_startAngle), cos(m_endAngle));
			_frame.sides[SIDE::RIGHT] = m_relativeCenter.x + (t < 0.0f ? m_innerRadius * t : m_outerRadius * t);
		}
		if( isAngleInRange(PI) )
			_frame.sides[SIDE::LEFT] = m_relativeCenter.x - m_outerRadius;
		else {
			float t = min(cos(m_startAngle), cos(m_endAngle));
			_frame.sides[SIDE::LEFT] = m_relativeCenter.x + (t > 0.0f ? m_innerRadius * t : m_outerRadius * t);
		}

		// Make the center position relative
		m_selfFrame = &_frame;
		m_relativeCenter -= Coord2(m_selfFrame->left(), m_selfFrame->bottom());
	}

}} // namespace ca::gui
