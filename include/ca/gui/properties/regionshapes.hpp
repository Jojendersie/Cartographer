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

	/// Use a bitmap mask to detect click events.
	class MaskRegion : public IRegion
	{
	public:
		virtual bool isMouseOver(const Coord2& _mousePos) const override;
	};

}} // namespace ca::gui
