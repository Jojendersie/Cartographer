#pragma once

#include "coordinate.hpp"

namespace ca { namespace gui {

	class IAnchorProvider;
	class IAnchorable;


	/// Describes of a boundary is positioned relative to another reference frame.
	class Anchor
	{
	private:
		/// The reference frame of a different widget.
		const IAnchorProvider* m_reference = nullptr;
		/// The widget to which the anchor belongs (must be set).
		/// If the reference changed, the attached component will be changed automatically.
		IAnchorable* m_self = nullptr;
		/// Linked list through all anchors that reference the same frame. If the
		/// respective widget gets deleted at cleans up the chain to avoid loose references.
		Anchor* m_next = nullptr;
		Anchor* m_prev = nullptr;
		friend class IAnchorProvider;
	public:
		/// The position within the frame (0.0=left/bottom and 1.0=right/top)
		Coord relativePosition = 0.0f;
		/// Distance to the point described by reference frame and relative position
		Coord absoluteDistance = 0.0f;

		Anchor(IAnchorable* _self) : m_self(_self) {}
		~Anchor();
		void detach();								///< Memory management of the anchorpoint (detach if host is gone)
		/// Link this anchor to a provider using an absolute reference position.
		/// \details Automaticall calls the provider's link method and detaches from previous providers.
		/// \param [in] _targetPosition Position on the target (or outside) that shall be kept in relative
		///		sync with the _target.
		/// \param [in] _anchorPosition The absolute position of the anchor (the difference to
		///		the _targetPosition defines the absolute distance.
		void attach(const IAnchorProvider* _target, Coord _targetPosition, Coord _anchorPosition, int _dimension);
		float getPosition(int _dimension) const;	///< Compute the current position. Input: the dimension 0=horizontal, 1=vertical
		/// Get the component to which this anchor belongs.
		IAnchorable* self() { return m_self; }
		/// Get the target to which this anchor is linked or nullptr.
		const IAnchorProvider* reference() const { return m_reference; }
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

		/// Special valuen to stear setAnchors()
		static constexpr Coord IGNORE = ei::INF;		///< Keep the respective anchor as is
		static constexpr Coord CLEAR = -ei::INF;		///< Detach the respective anchor if it is attached
	};


	/// Anything that has to do with positions can trigger updates when it is changed.
	class IPositionable
	{
	public:
		virtual void onExtentChanged() = 0;
	};


	/// Interface for classes to which anchors can be attached. The anchor provider
	/// allows queries of positions with respect to some object specific space of
	/// the deriving class.
	/// 
	/// Further it manges the anchor change on extent changes.
	class IAnchorProvider : public IPositionable
	{
		mutable Anchor m_anchorListStart;	///< Dummy anchor for the list to all connected widgets for update and memory management.
	public:
		IAnchorProvider() : m_anchorListStart(nullptr) {}
		~IAnchorProvider();

		/// Determine an absolute coordinate from a relative coordinate with respect
		/// to the provider.
		virtual Coord getPosition(int _dimension, float _relativePos) const = 0;

		/// Compute the relative position from an absolute one.
		virtual float getRelativePosition(int _dimension, Coord _position) const = 0;

		/// Couple an anchor so it will be updated on size changes and cleaned up automatically
		/// if this component is deleted.
		/// This method does not initialze the anchors position.
		void linkAnchor(Anchor& _anchor) const;

		/// Trigger the update of connected anchors.
		void onExtentChanged();
	};


	class IAnchorable : public IPositionable
	{
		bool m_resizeInProgress;			///< A lock to avoid recursive update if there are cyclic dependencies between elements. This must be used by derived refitToAnchors() methods!
		bool m_anchoringEnabled;			///< If disabled this will ignore notifications of anchor changes.

	public:
		IAnchorable();

		/// Check and set the resize flag. Returns true if refitting may take
		/// place. False is returned when anchoring is disabled or a recursive
		/// resize is detected.
		bool startRefit();

		/// Unlock component for next resizing event
		void endRefit();

		/// Recompute own positioning, because one of the anchors has changed.
		/// \details Must call startRefit() at the beginning and terminate on false.
		///	At the end endRefit() is necessary as well.
		virtual void refitToAnchors() = 0;

		/// Recompute the anchors without changing the own position.
		virtual void resetAnchors() = 0;

		bool isAnchoringEnabled() const { return m_anchoringEnabled; }
		void setAnchorable(bool _enable);
	};

}} // namespace ca::gui