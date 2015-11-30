#pragma once

#include "anchorable.hpp"
#include <memory>

namespace ca { namespace gui {

	/// A component which provides a set of anchors. These must be repositioned on each position
	/// or size change of the element.
	class IAnchorProvider
	{
	public:
		virtual ~IAnchorProvider() {}

		/// Recompute positions of all anchors based on the own reference frame.
		virtual void replaceAnchors(const class RefFrame& _selfFrame) = 0;

		static bool someAnchorChanged() { return m_someChanged; }
		static void resetChangedStatus() { m_someChanged = false; }
	protected:
		static bool m_someChanged;
	};

	/// Implementation of an anchor provider with one anchor per side.
	class BorderAnchorProvider : public IAnchorProvider
	{
	public:
		BorderAnchorProvider();
		~BorderAnchorProvider();

		virtual void replaceAnchors(const class RefFrame& _selfFrame) override;

		AnchorPtr getAnchor(SIDE::Val _side) const { return m_anchors[_side]; }
	private:
		std::shared_ptr<AnchorPoint> m_anchors[4];
	};

}} // namespace ca::gui