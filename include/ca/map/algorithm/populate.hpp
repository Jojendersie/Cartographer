#pragma once

#include "ca/map/map/grid.hpp"
#include "ca/map/map/iterator.hpp"
#include <ei/stdextensions.hpp>

namespace details {
	struct CellInfo
	{
		ca::map::GridCoord coord;
		int space;

		bool operator < (const CellInfo& _rhs) const
		{
			return space < _rhs.space;
		}

		bool operator == (const CellInfo& _rhs) const
		{
			return coord == _rhs.coord;
		}
	};
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
		int _minSpaceRadius, int _spaceAmount, int _forceNum,
		std::function<bool(typename const MapT::TCellType&)> _isEmpty,
		std::function<void(const GridCoord&)> _spawn)
	{
		// Determine free space for all tiles (regardless of threshold).
		pa::PriorityQueue<::details::CellInfo> queue;
		for(typename MapT::SeqIterator it = _map.begin(); it; ++it) if(!_isEmpty(it.dat()))
		{
			::details::CellInfo cinfo;
			cinfo.coord = it.coord();
			cinfo.space = 0;
			if(_minSpaceRadius > 0)
			{
				for(auto n = NeighborIterator<MapT::TGridType>(it.coord(), _minSpaceRadius); n; ++n)
				{
					typename MapT::TCellType* ncell = _map.find(n.coord());
					if(ncell && !_isEmpty(*ncell))
						cinfo.space--; // Min-heap -> use negative numbers
				}
			}
			queue.add(cinfo);
		}

		// Fill a set for poisson disc sampling using the priorised elements.
		// Either take all elements which fulfil the space requirement or more,
		// if _forceNum could not be satisfied otherwise.
		pa::HashSet<GridCoord> validTiles;	// Valid in space and poisson distance
		pa::HashSet<GridCoord> freeTiles;	// Fulfil space condition as much as possible and are not used yet.
		pa::HashSet<GridCoord> usedTiles;
		while(!queue.empty() && (abs(queue.min().space) >= _spaceAmount || int(validTiles.size()) < _forceNum))
		{
			// If the forceNum forced the usage of a too small tile, use all other tiles
			// of the same size too (they are not worse).
			_spaceAmount = min(_spaceAmount, abs(queue.min().space));

			freeTiles.add(queue.min().coord);
			validTiles.add(queue.popMin().coord);
		}

		// Remove tiles around seed.
		usedTiles.add(_seed);
		for(auto n = NeighborIterator<MapT::TGridType>(_seed, _minDistance); n; ++n)
			validTiles.remove(n.coord());

		int numCreated = 0;
		while(!validTiles.empty())
		{
			// Take a random tile from the validList. Since hashes are random
			// we simply take the first element.
			GridCoord coord = validTiles.begin().value();
			validTiles.remove(coord);
			freeTiles.remove(coord);
			usedTiles.add(coord);
			++numCreated;
			_spawn(coord);

			// Remove all the elements which are too close to the new element.
			// TODO: improve by using visible range iterator.
			for(auto n = NeighborIterator<MapT::TGridType>(coord, _minDistance); n; ++n)
				validTiles.remove(n.coord());
		}

		// Fill with random unused tiles from the freeTiles list. For each element
		// we want to make sure it has the larget possible distance to all spawned tiles.
		// First we create a priority queue where the radius of clearance is the (negative) priority.
		for(auto it : freeTiles)
		{
			::details::CellInfo cinfo;
			cinfo.coord = it;
			cinfo.space = -1000000;
			for(auto itUsed : usedTiles)
				cinfo.space = -min(-cinfo.space, distance<MapT::TGridType>(it, itUsed));
			queue.add(cinfo);
		}
		// Now spawn the first element from the queue and update the priorities of all others.
		// All tiles with the same priority are nicely randomized, because we took them from an
		// hash map.
		while(numCreated < _forceNum)
		{
			GridCoord coord = queue.popMin().coord;
			_spawn(coord);
			++numCreated;
			// Update distances for the remaining elements in the queue.
			for(auto& it : queue)
				it.space = -min(-it.space, distance<MapT::TGridType>(it.coord, coord));
			queue.heapify();
		}

		return numCreated;
	}

}} // namespace ca::map