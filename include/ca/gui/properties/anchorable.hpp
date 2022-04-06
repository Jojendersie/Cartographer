#pragma once

#include "coordinate.hpp"
#include "refframe.hpp"

namespace ca { namespace gui {

	class AnchorFrame;
	class Anchorable;

	/// Describes of a boundary is positioned relative to another reference frame.
	class Anchor
	{
	private:
		/// The reference frame of a different widget.
		const AnchorFrame* reference = nullptr;
		/// The widget to which the anchor belongs (must be set).
		/// If the reference changed, the attached component will be changed automatically.
		Anchorable* self = nullptr;
		/// Linked list through all anchors that reference the same frame. If the
		/// respective widget gets deleted at cleans up the chain to avoid loose references.
		Anchor* next = nullptr;
		Anchor* prev = nullptr;
		/// The position within the frame (0.0=left/bottom and 1.0=right/top)
		Coord relativePosition = 0.0f;
		/// Distance to the point described by reference frame and relative position
		Coord absoluteDistance = 0.0f;

		friend class Anchorable;
		friend class AnchorFrame;
	public:
		~Anchor();
		void detach();								///< Memory management of the anchorpoint (detach if host is gone)
		float getPosition(int _a, int _b) const;	///< Compute the current position. Input: the two indices of a=left/bottom, r=top/right
	};



	/// An extended reference frame to which anchors can be attached.
	class AnchorFrame : public RefFrame
	{
	public:
		~AnchorFrame();
		/// Couple an anchor so it will cleaned up automatically if this component is deleted.
		void linkAnchor(Anchor& _anchor) const;

	protected:
		/// React to the change by recursively updating all anchored components.
		void onExtentChanged(const CHANGE_FLAGS::Val _changes) override;
	private:
		mutable Anchor m_anchorListStart;	///< Dummy anchor for memory management.
	};



	/// Special valuen to stear setAnchors()
	constexpr Coord IGNORE_ANCHOR = ei::INF;		///< Keep the respective anchor as is
	constexpr Coord CLEAR_ANCHOR = -ei::INF;		///< Detach the respective anchor if it is attached

	enum class AutoAnchorMode
	{
		ABSOLUTE,		///< Take the four borders of the reference frame as anchor targets
		RELATIVE,		///< Take the four borders of the current element as anchor targets
		SRC_CENTER,		///< Set the bottom left anchors to the current element center (that will move relative to the reference frame)
		DST_CENTER,		///< Set the bottom left anchors to the target element center (that will move relative to the reference frame)
	};

	/// A mask value for auto anchoring
	struct ANCHOR
	{
		using Val = uint32;
		static constexpr uint32 LEFT = 1 << SIDE::LEFT;
		static constexpr uint32 RIGHT = 1 << SIDE::RIGHT;
		static constexpr uint32 BOTTOM = 1 << SIDE::BOTTOM;
		static constexpr uint32 TOP = 1 << SIDE::TOP;
		static constexpr uint32 HORIZONTAL = LEFT | RIGHT;
		static constexpr uint32 VERTICAL = BOTTOM | TOP;
		static constexpr uint32 ALL = HORIZONTAL | VERTICAL;
		static constexpr uint32 POSITION = BOTTOM | LEFT;
	};

	/// Anchorable classes are moved/resized dependent on the movement of anchor points.
	/// \details Anchor points are provided by other components. An anchorable
	///		component can be moved or resized on change of its anchor points. It has four
	///		anchors, one on each side, which can have different restrictions for the degrees of
	///		freedom.
	///
	///		If only one anchor in a dimension (horizontal or vertical) is set, the component
	///		will keep its size in this dimension. Each movement of the anchor will result
	///		in a movement of the entire component.
	/// 
	///		If both anchors in one dimension are set, the two respective sides will independently
	///		satisfy their constraints.
	class Anchorable
	{
	public:

		/// Make a reference frame anchorable.
		/// \param [in] _selfFrame The reference frame which is modified based on the anchoring.
		Anchorable(RefFrame* _selfFrame);

		/// Set all anchors at once.
		/// \details For each of the four sides an absolute reference point within
		///		the respective dimension can be specified. These references are
		///		linked relative to the target frame. Using IGNORE_ANCHOR single
		///		anchors can be ignored (will keep their current anchoring).
		///
		///		By using the current frame's sides directly, a fully relative
		///		anchoring is achieved (points will stay proportionally at the same
		///		position to the _targetFrame).
		///		By using the target frame's sides directly, a fully absolute
		///		anchoring is achived (points will keep constant distance to the
		///		boundaries of the _targetFrame).
		///		Anything in between is (or outside the target and source frame
		///		is valid as well.
		/// \param [in] _targetFrame New reference or nullptr to release all anchors
		///		that do not have a value of IGNORE_ANCHOR.
		/// \param [in] _<dim>target IGNORE_ANCHOR or an absolute coordinate
		///		for each of the four edges of the current frame. The absolute distance
		///		between the current boundary position and the target points will be
		///		kept. Also see details.
		void setAnchors(
			const AnchorFrame* _targetFrame,
			Coord _leftTarget,
			Coord _bottomTarget,
			Coord _rightTarget,
			Coord _topTarget
		);

		/// Calls setAnchors() with some default arguments based on mode.
		/// \param [in] _targetFrame New reference or nullptr to release all anchors
		///		that do not have a value of IGNORE_ANCHOR.
		/// /// \param [in] _mode Select between absolute, relative and center position anchoring.
		/// \param [in] _mask Only change those anchors in mask, ignore the others
		void setAnchors(
			const AnchorFrame* _targetFrame,
			AutoAnchorMode _mode,
			ANCHOR::Val _mask = ANCHOR::ALL
		);

		/// Recompute relative positioning. E.g. if a component was moved manually.
		void resetAnchors();

		/// Resize/renew position the object dependent on the current anchor points
		/// \return True if any property was changed by the refit method.
		void refitToAnchors();

		// TODO: enable/disable
		bool isAnchoringEnabled() const { return m_anchoringEnabled; }
		void setAnchorable(bool _enable);

	private:
		Anchor m_anchor[4];				///< The four reference points (l, r, t, b)
		RefFrame* m_selfFrame; // TODO: implement propert copy/move or make sure they cant be used
		bool m_anchoringEnabled;
	};
}} // namespace ca::gui