#pragma once

#include "coordinate.hpp"

namespace cag {

	class IRegion
	{
	public:
		/// Check if the mouse position is over this element.
		/// \param [in] _mousePos Position of the cursor in screenspace [0,1]^2.
		virtual bool isMouseOver(const Coord& _mousePos) = 0;
	};

	/// Basic rectangular area which is used as reference from all widgets.
	class RefFrame: public IRegion
	{
	public:
		float left;
		float right;
		float bottom;
		float top;

		/// Check if the mouse cursor is on this reference frame.
		/// \param [in] _mousePos Position of the cursor in screenspace [0,1]^2.
		virtual bool isMouseOver(const Coord& _mousePos) override;
	};

	/// A general purpose enum to define positioning schemes
	enum class SIDE
	{
		LEFT = 0,
		RIGHT = 1,
		BOTTOM = 2,
		TOP = 3
	};

} // namespace cag