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
		static uint32 s_globalGeomVersion;	///< A continiously increased number to manage the update process.
		uint32 m_geomVersion = 0;			///< Current version used to manage updates (to avoid cyclic updates).

		bool m_anchoringEnabled;			///< If disabled this will ignore notifications of anchor changes.

	public:
		IAnchorable();

		/// Recompute own positioning, because one of the anchors has changed.
		/// \details matches current geometry version.
		virtual void refitToAnchors();

		/// Recompute the anchors without changing the own position.
		/// \details matches current geometry version.
		virtual void resetAnchors();

		/// Return the version number of global changes
		static uint32 getGlobalGeomVersion() { return s_globalGeomVersion; }
		uint32 getGeomVersion() const { return m_geomVersion; }
		/// Increase global version and assign to this object (use for primary updates).
		void increaseGeomVersion() { m_geomVersion = ++s_globalGeomVersion; }
		/// Set object version to the global one (used in reactions to refitting).
		void matchGeomVersion() { m_geomVersion = s_globalGeomVersion; }

		bool isAnchoringEnabled() const { return m_anchoringEnabled; }
		void setAnchorable(bool _enable);
	};

}} // namespace ca::gui