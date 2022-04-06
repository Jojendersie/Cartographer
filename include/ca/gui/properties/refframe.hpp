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

		static constexpr char const * STR_NAMES[] = {
			"LEFT",
			"RIGHT",
			"BOTTOM",
			"TOP",
			"CENTER"
		};
	};


	/// Basic rectangular area which is used as reference from all widgets.
	class RefFrame: public IRegion
	{
		union {
			float m_sides[4];		///< An array of the four side coordinates sorted after SIDE
			ei::Rect2D m_rect;
		};

		static uint32 geomVersion;	///< A continiously increased number to manage the update process.
		uint32 m_geomVersion = 0;	///< Current version used to manage updates (to avoid cyclic updates).

	public:
		RefFrame() : m_rect{ei::Vec2{0.0f}, ei::Vec2{1.0,1.0f}} {}
		RefFrame(float _l, float _r, float _b, float _t);

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

		bool operator != (const RefFrame& _rhs) const;
		bool operator == (const RefFrame& _rhs) const;

		/// Set all the sizes of the references frame. Returns a true to indicate changes.
		virtual bool setFrame(const float _l, const float _b, const float _r, const float _t);

		/// Return the increasing version number of global changes
		static uint32 getGlobalGeomVersion() { return geomVersion; }
		uint32 getGeomVersion() const { return m_geomVersion; }

		/// Optional method to react to resizing or repositioning events
		virtual void onExtentChanged() {}
	protected:
		/// Overwrite the reference frame without triggering onResize or geometry version changes
		void silentSetFrame(const float _l, const float _b, const float _r, const float _t);
		friend class Anchorable;
	};

}} // namespace ca::gui