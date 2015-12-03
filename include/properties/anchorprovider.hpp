#pragma once

#include "anchorable.hpp"
#include <memory>
#include <vector>

namespace ca { namespace gui {

	/// A component which provides a set of anchors. These must be repositioned on each position
	/// or size change of the element.
	class IAnchorProvider
	{
	public:
		virtual ~IAnchorProvider() {}

		/// Recompute positions of all anchors based on the own reference frame.
		virtual void recomputeAnchors(const class RefFrame& _selfFrame) = 0;

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

		virtual void recomputeAnchors(const class RefFrame& _selfFrame) override;

		AnchorPtr getAnchor(SIDE::Val _side) const { return m_anchors[_side]; }
	private:
		std::shared_ptr<AnchorPoint> m_anchors[4];
	};

	/// Implementation of an anchor provider with a horizontal and a vertical subdivision.
	/// \detais If only one anchor is created in a direction, it is centered. Otherwise the borders
	///		ar the anchors with min/max index.
	class GridAnchorProvider : public IAnchorProvider
	{
	public:
		GridAnchorProvider(int _gridDimX, int _gridDimY);
		~GridAnchorProvider();

		virtual void recomputeAnchors(const class RefFrame& _selfFrame) override;

		AnchorPtr getHAnchor(int _index) const { return m_hAnchors[_index]; }
		AnchorPtr getVAnchor(int _index) const { return m_vAnchors[_index]; }
	private:
		std::vector<std::shared_ptr<AnchorPoint>> m_hAnchors;
		std::vector<std::shared_ptr<AnchorPoint>> m_vAnchors;
	};

}} // namespace ca::gui