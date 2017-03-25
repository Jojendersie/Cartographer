#pragma once

#include "ca/gui/properties/refframe.hpp"

namespace ca { namespace gui {

	/// Use the reference frame fitted ellipse to detect click events.
	class EllipseRegion : public IRegion
	{
	public:
		/// \param [in] _selfFrame The reference frame for the clickable area
		EllipseRegion(const RefFrame* _selfFrame);

		virtual bool isMouseOver(const Coord2& _mousePos) const override;
	private:
		const RefFrame* m_selfFrame;
	};

	/// A region for a segment of a ring (or 2D donut-segment) for circular menus.
	/// The circle center can be anywhere and does not need to be inside the reference frame.
	/// However, to guarantee that the described segment is inside the frame use the fit method.
	class AnnulusSegmentRegion : public IRegion
	{
	public:
		/// \param [in] _innerRadius .
		/// \param [in] _startAngle Any angle in radiants to define the start of the segment.
		/// \param [in] _endAngle Any angle in radiants to define the end of the segment.
		///		Must be different from start. To define the entire ring use 0 and 2pi for the
		///		two numbers.
		AnnulusSegmentRegion(const Coord2& _center,
			Coord _innerRadius,
			Coord _outerRadius,
			float _startAngle,
			float _endAngle);

		virtual bool isMouseOver(const Coord2& _mousePos) const override;
	private:
		const RefFrame* m_selfFrame;
		Coord2 m_relativeCenter;	///< Center of the annulus relative to the ref-frame bottom left coordinate. Set from attach(). The absolute center previously to the attach() call.
		// TODO: react on rescaling of the refframe
		//Coord2 m_relativeSize;		///< Scaling factor for the ref-frame to get the 
		Coord m_innerRadiusSq;
		Coord m_innerRadius;
		Coord m_outerRadiusSq;
		Coord m_outerRadius;
		float m_startAngle;
		float m_endAngle;

		/// Change the coordinates of the given frame to fit the segment as close as possible.
		void attach(RefFrame& _frame) override;

		bool isAngleInRange(float _angle) const {
			if(m_startAngle < m_endAngle)
				return m_startAngle <= _angle && _angle <= m_endAngle;
			return m_startAngle <= _angle || _angle <= m_endAngle;
		}
	};

	/// Use a bitmap mask to detect click events.
	class MaskRegion : public IRegion
	{
	public:
		virtual bool isMouseOver(const Coord2& _mousePos) const override;
	};

}} // namespace ca::gui
