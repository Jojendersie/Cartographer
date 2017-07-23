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
		pa::PriorityQueue<::details::CellInfo> tiles;
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
			tiles.add(cinfo);
		}

		// Fill a set for poisson disc sampling using the priorised elements.
		// Either take all elements which fulfil the space requirement or more,
		// if _forceNum could not be satisfied otherwise.
		pa::HashSet<::details::CellInfo> validTiles;	// Valid in space and poisson distance
		pa::HashSet<::details::CellInfo> freeTiles;	// Fulfil space condition as much as possible and are not used yet.
		while(!tiles.empty() && (abs(tiles.min().space) >= _spaceAmount || int(validTiles.size()) < _forceNum))
		{
			freeTiles.add(tiles.min());
			validTiles.add(tiles.popMin());
		}

		// Remove tiles around seed.
		for(auto n = NeighborIterator<MapT::TGridType>(_seed, _minDistance); n; ++n)
		{
			::details::CellInfo cinfo;
			cinfo.coord = n.coord();
			validTiles.remove(cinfo);
		}

		int numCreated = 0;
		while(!validTiles.empty())
		{
			// Take a random tile from the validList. Since hashes are random
			// we simply take the first element.
			::details::CellInfo cinfo = validTiles.begin().value();
			validTiles.remove(cinfo);
			freeTiles.remove(cinfo);
			++numCreated;
			_spawn(cinfo.coord);

			// Remove all the elements which are too close to the new element.
			// TODO: improve by using visible range iterator.
			for(auto n = NeighborIterator<MapT::TGridType>(cinfo.coord, _minDistance); n; ++n)
			{
				cinfo.coord = n.coord();
				validTiles.remove(cinfo);
			}
		}

		// Fill with random unused tiles from the freeTiles list.
		while(numCreated < _forceNum)
		{
			for(auto it : freeTiles)
			{
				_spawn(it.coord);
				++numCreated;
			}
		}

		return numCreated;
	}

}} // namespace ca::map