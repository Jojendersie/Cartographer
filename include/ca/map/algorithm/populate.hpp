#pragma once

#include "ca/map/map/grid.hpp"
#include "ca/map/map/iterator.hpp"
#include "ca/map/algorithm/utilities.hpp"
#include <ei/stdextensions.hpp>

namespace details {
	struct CellInfo
	{
		ca::map::GridCoord coord;
		float spaceCost;
		float distCost;

		bool operator < (const CellInfo& _rhs) const
		{
			return cost() < _rhs.cost();
		}

		bool operator == (const CellInfo& _rhs) const
		{
			return coord == _rhs.coord;
		}

		float cost() const { return spaceCost + distCost; }
	};

	// Helper which changes the distCost of all cellinfos within a given range.
	template<typename MapT>
	static void updateCostsInRange(const MapT& _map,
		ca::map::ReachableSet& _reachableTiles,
		const ca::map::GridCoord& _from, int _maxDistance,
		const std::function<bool(typename const MapT::TCellType&)>& _isEmpty,
		ca::pa::HashPriorityQueue<CellInfo>& _queue)
	{
		findReachable(_map, _reachableTiles, _from, _maxDistance, _isEmpty);
		for(auto it : _reachableTiles)
		{
			auto handle = _queue.find(CellInfo{it.key(), 0.0f, 0.0f});
			if(handle)
			{
				float distCost = (_maxDistance - it.data() + 1) / float(_maxDistance);
				handle->distCost = ei::max(distCost, handle->distCost);
				_queue.priorityChanged(handle);
			}
		}
	}
}

namespace std {
	template <>
	struct hash<details::CellInfo>
	{
		using argument_type = details::CellInfo;
		using result_type = std::size_t;

		std::size_t operator()(const details::CellInfo& _key) const
		{
			return hash<ei::IVec2>()(_key.coord);
		}
	};

}

namespace ca { namespace map {

	/// Fill the map with some objects/enemies/... using poisson disc sampling.
	/// \param [in] _minDistance Minimum distance to other objects and the seed.
	/// \param [in] _minSpaceRadius Only spawn on tiles which have a minimum number
	///		of _spaceAmount non-empty cells in a radius of _minSpaceRadius.
	/// \param [in] _forceNum Always create at least _forceNum whatever, even
	///		if some space condition is violated.
	/// \param _spawn A function to spawn whatever at a sampled position
	/// \returns Number of generated whatever.
	template<typename MapT>
	int populate(MapT& _map, const GridCoord& _seed, int _minDistance,
		int _minSpaceRadius, int _spaceAmount,
		std::function<bool(typename const MapT::TCellType&)> _isEmpty,
		std::function<void(const GridCoord&)> _spawn)
	{
		// Determine free space for all tiles and use only the good ones.
		pa::HashSet<GridCoord> validTiles;
		for(typename MapT::SeqIterator it = _map.begin(); it; ++it) if(!_isEmpty(it.dat()))
		{
			int space = 0;
			if(_minSpaceRadius > 0)
			{
				for(auto n = NeighborIterator<MapT::TGridType>(it.coord(), _minSpaceRadius); n; ++n)
				{
					typename MapT::TCellType* ncell = _map.find(n.coord());
					if(ncell && !_isEmpty(*ncell))
						space++;
				}
			}
			if(space >= _spaceAmount)
				validTiles.add(it.coord());
		}

		// Remove tiles around seed.
		for(auto n = NeighborIterator<MapT::TGridType>(_seed, _minDistance); n; ++n)
			validTiles.remove(n.coord());

		// Take random elements from the valid set and remove too close from the set each time.
		int numCreated = 0;
		ReachableSet reachableTiles; // Tmp memory for reuse
		while(!validTiles.empty())
		{
			GridCoord coord = validTiles.begin().value();
			++numCreated;
			_spawn(coord);
			validTiles.remove(coord);
			// Remove all the elements which are too close to the new element.
			findReachable(_map, reachableTiles, coord, _minDistance, _isEmpty);
			for(auto it : reachableTiles)
				validTiles.remove(it.key());
		}

		return numCreated;
	}

	// New layout:
	// * Fill priority queue with costs = number of free space violations
	// * Mark all things close to the start tile
	// * Take element from queue
	// * Update priority of neighborhood:
	//   - find using hashmap
	//   - add an increasing cost the closer the element is
	template<typename MapT>
	int populate2(MapT& _map, const GridCoord& _seed, int _minDistance,
		int _minSpaceRadius, int _requiredSpace, int _forceNum,
		std::function<bool(typename const MapT::TCellType&)> _isEmpty,
		std::function<void(const GridCoord&)> _spawn)
	{
		using namespace ::details;
		eiAssert(numCellsInRange<MapT::TGridType>(_minSpaceRadius) >= _requiredSpace, "Required space condition cannot be met.");
		// A queue combined with a hashmap to find elements in the queue
		pa::HashPriorityQueue<CellInfo> queue(2000);
		// Get the free space of all tiles
		for(typename MapT::SeqIterator it = _map.begin(); it; ++it) if(!_isEmpty(it.dat()) && it.coord() != _seed)
		{
			int space = 0;
			if(_minSpaceRadius > 0)
			{
				for(auto n = NeighborIterator<MapT::TGridType>(it.coord(), _minSpaceRadius); n; ++n)
				{
					typename MapT::TCellType* ncell = _map.find(n.coord());
					if(ncell && !_isEmpty(*ncell))
						space++;
				}
			} else space = 1;
			::details::CellInfo cinfo;
			cinfo.coord = it.coord();
			// Relative cost 0-100% for space violation
			cinfo.spaceCost = ei::max(_requiredSpace - space, 0) / ei::max(float(_requiredSpace), 1e-10f);
			cinfo.distCost = 0.0f;
			queue.add(cinfo);
		}

		// Add costs when close to start.
		ReachableSet reachableTiles;
		updateCostsInRange(_map, reachableTiles, _seed, _minDistance, _isEmpty, queue);

		// Spawn elements
		int numCreated = 0;
		while(!queue.empty()
			&& !(numCreated >= _forceNum && queue.min().cost() > 0.0f) // Still valid tiles, or required tiles
		) {
			auto cinfo = queue.popMin();
			_spawn(cinfo.coord);
			++numCreated;
			// Increase costs of all close by tiles, based on free radius.
			updateCostsInRange(_map, reachableTiles, cinfo.coord, _minDistance, _isEmpty, queue);
		}

		return numCreated;
	}

}} // namespace ca::map