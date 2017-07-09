#pragma once

#include "coordinate.hpp"
#include "refframe.hpp"
#include <memory>

namespace ca { namespace gui {

	/// A position which can be used as reference for any anchor of an anchorable object.
	struct AnchorPoint
	{
		const void* host;		///< Who is responsible for this point? If the host object is deleted it must set this to nullptr. Then anchors are released automatic.
		Coord position;			///< Current position.
		AnchorPoint(const void* _host) : host(_host) {}
	};

	typedef std::shared_ptr<const AnchorPoint> AnchorPtr;

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
			NO_RESIZE,			///< Try to set the position such that quadratic deviation between all anchor-positions is minimized.
			PREFER_MOVE,		///< If not anchored on either side move the object and keep its size.
			PREFER_RESIZE,		///< No matter if the opposite side is anchored resize the object.
		};

		/// Make a reference frame anchorable.
		/// \param [in] _selfFrame The reference frame which is modified based on the anchoring.
		Anchorable(RefFrame* _selfFrame);

		/// Attach/detach an anchor on a side. To detach pass a nullptr.
		/// \details Setting the anchor computes a relativ anchor for the current state. I.e. the
		///		position of the new anchor is fixed relativ to the anchor-point.
		/// \param [in] _side Which anchor should be reseted?
		/// \param [in] _anchorPoint New reference point or nullptr to fix or release the anchor.
		void setAnchor(SIDE::Val _side, AnchorPtr _anchorPoint);

		/// Automatically attach all four anchor points to the closest anchors in the provider
		void autoAnchor(const class IAnchorProvider* _anchorProvider);

		/// Recompute relative positioning. E.g. if a component was moved manually.
		void resetAnchors();

		void setHorizontalAnchorMode(Mode _mode);
		void setVerticalAnchorMode(Mode _mode);
		/// Set horizontal and vertical anchor mode simultaneously
		void setAnchorModes(Mode _mode);
		void setAnchorModes(Mode _horizontalMode, Mode _verticalMode);

		/// Resize/renew position the object dependent on the current anchor points
		/// \return True if any property was changed by the refit method.
		bool refitToAnchors();

		// TODO: enable/disable
		bool isAnchoringEnabled() const { return m_anchoringEnabled; }
		void setAnchorable(bool _enable) { m_anchoringEnabled = _enable; }
	private:
		struct Anchor
		{
			AnchorPtr reference;			///< Any anchor point
			Coord relativePosition;			///< Difference of the component's anchor to the reference point.
			Anchor() : reference(nullptr) {}
			void checkReference();			///< Memory management of the anchorpoint (detach if host is gone)
		};
		Anchor m_anchors[4];			///< The four reference points (l, r, t, b)
		RefFrame* m_selfFrame;
		Mode m_horizontalMode;			///< The component can be rescaled in horizontal direction to satisfy left/right anchors
		Mode m_verticalMode;			///< The component can be rescaled in vertical direction to satisfy bottom/top anchors
		bool m_anchoringEnabled;
	};
}} // namespace ca::gui