#pragma once

#include "coordinate.hpp"
#include "refframe.hpp"

namespace cag {

	/// A position which can be used as reference for any anchor of an anchorable object.
	struct AnchorPoint
	{
		void* host;			///< Who is responsible for this point? If the host object is deleted it must set this to nullptr. Then anchors are released automatic.
		Coord2 position;		///< Current position.
	};

	/// Anchorable classes are moved/resized dependent on the movement of anchor points.
	/// \details Anchor points are provided by other components and snapping grids. An anchorable
	///		component can be moved or resized on change of its anchor points. It has four
	///		anchors, one on each side, which can have different restrictions for the degrees of
	///		freedom.
	///
	///		A component may or may not be resized to satisfy the anchoring see Anchorable::Mode
	///		for more details.
	class Anchorable
	{
	public:
		/// A component may or may not be resized to satisfy the anchoring.
		enum Mode
		{
			NO_RESIZE,					///< Try to set the position such that quadratic deviation between all anchor-positions is minimized.
			PREFER_MOVE,				///< If not anchored on either side move the object and keep its size.
			PREFER_RESIZE,				///< No matter if the opposite side is anchored resize the object.
		};

		/// Make a reference frame anchorable.
		/// \param [in] _selfFrame The reference frame which is modified based on the anchoring.
		Anchorable(RefFrame* _selfFrame);

		/// Attach/detach an anchor on a side. To detach pass a nullptr.
		/// \details Setting the anchor computes a relativ anchor for the current state. I.e. the
		///		position of the new anchor is fixed relativ to the anchor-point.
		/// \param [in] _side Which anchor should be reseted?
		/// \param [in] _anchorPoint New reference point or nullptr to fix or release the anchor.
		void setAnchor(SIDE::Val _side, const AnchorPoint* _anchorPoint);

		/// Recompute relative positioning. E.g. if a component was moved manually.
		void resetAnchors();

		void setHorizontalAnchorMode(Mode _mode);
		void setVerticalAnchorMode(Mode _mode);

		/// Resize/renew position the object dependent on the current anchor points
		/// \return True if any property was changed by the refit method.
		bool refitToAnchors();
	private:
		struct Anchor
		{
			const AnchorPoint* reference;	///< Any anchor point
			Coord relativePosition;			///< Difference of the component's anchor to the reference point.
			Anchor() : reference(nullptr) {}
		};
		Anchor m_anchors[4];			///< The four reference points (l, r, t, b)
		RefFrame* m_selfFrame;
		Mode m_horizontalMode;			///< The component can be rescaled in horizontal direction to satisfy left/right anchors
		Mode m_verticalMode;			///< The component can be rescaled in vertical direction to satisfy bottom/top anchors
	};
} // namespace cag