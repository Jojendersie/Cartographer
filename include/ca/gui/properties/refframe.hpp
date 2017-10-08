#pragma once

#include "coordinate.hpp"
#include <ei/2dtypes.hpp>

namespace ca { namespace gui {

	class IRegion
	{
	public:
		/// Check if the mouse position is over this element.
		/// \param [in] _mousePos Position of the cursor in screen space [0,1]^2.
		virtual bool isMouseOver(const Coord2& _mousePos) const = 0;

	private:
		friend class Widget;
		/// Optional method which is called from Widget::setRegion() to fit the widgets
		/// reference frame to the region.
		/// The region must still be able to handle move and resize events of the reference frame.
		virtual void attach(class RefFrame& _selfFrame) {}
	};

	/// A general purpose enum to define positioning schemes
	/// \details To access general dimensions (i.e. x and y) in vectors you can use SIDE/2 as
	///		index.
	struct SIDE
	{
		enum Val
		{
			LEFT = 0,
			RIGHT = 2,
			BOTTOM = 1,
			TOP = 3,
			CENTER = 4,
		};
	};

	/// Basic rectangular area which is used as reference from all widgets.
	class RefFrame: public IRegion
	{
	public:
		RefFrame() {}
		RefFrame(float _l, float _r, float _b, float _t);

		union {
			float sides[4];	/// An array of the four side coordinates sorted after SIDE
			ei::Rect2D rect;
		};

		float left() const { return sides[SIDE::LEFT]; }
		float right() const { return sides[SIDE::RIGHT]; }
		float bottom() const { return sides[SIDE::BOTTOM]; }
		float top() const { return sides[SIDE::TOP]; }
		float width() const { return sides[SIDE::RIGHT] - sides[SIDE::LEFT]; }
		float height() const { return sides[SIDE::TOP] - sides[SIDE::BOTTOM]; }
		float horizontalCenter() const { return 0.5f * (sides[SIDE::RIGHT] + sides[SIDE::LEFT]); }
		float verticalCenter() const { return 0.5f * (sides[SIDE::TOP] + sides[SIDE::BOTTOM]); }
		ei::Vec2 center() const { return ei::Vec2(horizontalCenter(), verticalCenter()); }
		ei::Vec2 size() const { return ei::Vec2(width(), height()); }
		ei::Vec2 position() const { return ei::Vec2(left(), bottom()); }

		/// Check if the mouse cursor is on this reference frame.
		/// \param [in] _mousePos Position of the cursor in screen space [0,1]^2.
		virtual bool isMouseOver(const Coord2& _mousePos) const override;

		bool operator != (const RefFrame& _rhs) const;
		bool operator == (const RefFrame& _rhs) const;
	};

}} // namespace ca::gui