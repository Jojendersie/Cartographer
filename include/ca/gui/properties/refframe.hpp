#pragma once

#include "coordinate.hpp"
#include "anchorable.hpp"
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


	/// Basic rectangular area which is used as reference from all widgets.
	/// \details The reference frame can provid anchoring as well as being anchored
	///		on all four sides. A change of one of the anchors will be notified to the
	///		frame and immediatelly triggers its update (recursively).
	/// 
	///		If only one anchor in a dimension (horizontal or vertical) is set, the component
	///		will keep its size in this dimension. Each movement of the anchor will result
	///		in a movement of the entire component.
	/// 
	///		If both anchors in one dimension are set, the two respective sides will independently
	///		satisfy their constraints.
	class RefFrame: public IRegion, public IAnchorable, public IAnchorProvider
	{
		union {
			float m_sides[4];		///< An array of the four side coordinates sorted after SIDE
			ei::Rect2D m_rect;
		};

		Anchor m_anchor[4];			///< The four reference points (l, r, t, b)

	public:
		RefFrame(float _l, float _r, float _b, float _t);
		RefFrame() : RefFrame(0.0f, 0.0f, 1.0f, 1.0f) {}

		float left() const { return m_sides[SIDE::LEFT]; }
		float right() const { return m_sides[SIDE::RIGHT]; }
		float bottom() const { return m_sides[SIDE::BOTTOM]; }
		float top() const { return m_sides[SIDE::TOP]; }
		float width() const { return m_sides[SIDE::RIGHT] - m_sides[SIDE::LEFT]; }
		float height() const { return m_sides[SIDE::TOP] - m_sides[SIDE::BOTTOM]; }
		float horizontalCenter() const { return 0.5f * (m_sides[SIDE::RIGHT] + m_sides[SIDE::LEFT]); }
		float verticalCenter() const { return 0.5f * (m_sides[SIDE::TOP] + m_sides[SIDE::BOTTOM]); }
		ei::Vec2 center() const { return ei::Vec2(horizontalCenter(), verticalCenter()); }
		ei::Vec2 size() const { return ei::Vec2(width(), height()); }
		ei::Vec2 position() const { return ei::Vec2(left(), bottom()); }
		const ei::Rect2D& rectangle() const { return m_rect; }
		const float side(const int _idx) const { return m_sides[_idx]; }

		/// Check if the mouse cursor is on this reference frame.
		/// \param [in] _mousePos Position of the cursor in screen space [0,1]^2.
		virtual bool isMouseOver(const Coord2& _mousePos) const override;

		Coord getPosition(int _dimension, float _relativePos) const override;
		float getRelativePosition(int _dimension, Coord _position) const override;

		void onExtentChanged() override;

		bool operator != (const RefFrame& _rhs) const;
		bool operator == (const RefFrame& _rhs) const;

		/// Set all the sizes of the references frame. Returns a true to indicate changes.
		virtual bool setFrame(const float _l, const float _b, const float _r, const float _t);

		/// Set anchors such that each flagged side inside _mask is fixed to
		/// the closest edge of the _targetFrame,
		/// 
		/// \param [in] _targetFrame New reference or nullptr to release all anchors
		///		that do not have a value of IGNORE_ANCHOR.
		/// \param [in] _mask Only change those anchors in mask, ignore the others
		void setAutoAnchors(
			const RefFrame* _targetFrame,
			SIDE_FLAGS::Val _mask = SIDE_FLAGS::ALL
		);

		/// Get a reference point against which an anchor can be set.
		/// \param[in] _relativePosition A position in- or outside the frame.
		///		left/bottom == 0 and right/top == 1 (depending on which dimension
		///		this point belongs to.
		struct AnchorPoint {
			const IAnchorProvider* target;
			Coord2 position;
		};
		AnchorPoint getAnchorPoint(float _relativePosX, float _relativePosY) const;

		/// Get a reference point against which an anchor can be set from an absolute position.
		/// \param[in] _coord Absolute global position at which we want to anchor.
		///		Note that any change will be relative to this components as common.
		///		The only difference is in how we select the actual reference point.
		AnchorPoint getAnchorPoint(const Coord2& _coord) const;

		/// Connect one or multiple anchor of this widget to an
		///	anchor point from a different element.
		void setAnchors(SIDE_FLAGS::Val _mask, const AnchorPoint& _anchorPoint);

		/// Recompute relative positioning. E.g. if a component was moved manually.
		void resetAnchors() override;

		/// Resize/renew position the object dependent on the current anchor points
		void refitToAnchors() override;
	protected:
		/// Overwrite the reference frame without triggering onResize or geometry version changes
		void silentSetFrame(const float _l, const float _b, const float _r, const float _t);
	};

}} // namespace ca::gui