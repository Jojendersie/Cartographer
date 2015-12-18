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


		enum class SearchDirection
		{
			HORIZONTAL,		///< Search in x direction but don't care if the anchor is left or right
			LEFT,			///< Search only anchors left to the reference position
			RIGHT,			///< Search only anchors right to the reference position
			VERTICAL,		///< Search in y direction but don't care if the anchor is above or below
			DOWN,			///< Search only anchors below the reference position
			UP				///< Search only anchors above the reference position
		};

		/// From all provided horizontal anchors find the one closest to the given position.
		/// \param [in] _position Position from where to begin the search. Usually this is
		///		the point to be anchored in one dimension.
		/// \param [in] _direction Only consider anchors which lie this direction with respect
		///		to the _position. For more details see member descriptions of SearchDirection.
		/// \return The closest anchor in search direction or nullptr if there is none.
		virtual AnchorPtr findClosestAnchor(Coord _position, SearchDirection _direction) const = 0;

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
		virtual AnchorPtr findClosestAnchor(float _position, SearchDirection _direction) const override;

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
		virtual AnchorPtr findClosestAnchor(float _position, SearchDirection _direction) const override;

		AnchorPtr getHAnchor(int _index) const { return m_hAnchors[_index]; }
		AnchorPtr getVAnchor(int _index) const { return m_vAnchors[_index]; }
	private:
		std::vector<std::shared_ptr<AnchorPoint>> m_hAnchors;
		std::vector<std::shared_ptr<AnchorPoint>> m_vAnchors;
	};

}} // namespace ca::gui