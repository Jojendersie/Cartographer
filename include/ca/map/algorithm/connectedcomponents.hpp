#pragma once

#include "ca/map/map/grid.hpp"

namespace ca { namespace map {

	namespace details
	{
		// Implicit castable marker <-> bool. This avoids undefined initialization
		// and std::vector<bool> specializations.
		struct Marker
		{
			Marker() : mark(false) {}
			Marker(bool b) : mark(b) {}
			operator bool () const { return mark; }
			bool mark;
		};

		// Scanline flood fill:
		// - mark all things in one row by looping left and right, stop if an empty
		//	or marked cell is reached.
		// - go one line up and down and loop over all neighbors, seeing the marked range.
		// - for each unmarked, unempty neighbor call recursively.
		template<typename MapT, typename MarkerT>
		int scanlineFloodFill(const MapT& _map, Grid<MapT::TGridType, MarkerT>& _markers, MarkerT _mark, const GridCoord& _seed, const std::function<bool(typename const MapT::TCellType&)>& _isEmpty)
		{
			auto * cell = _map.find(_seed);
			if(!cell) return 0;
			if(_isEmpty(*cell)) return 0;
			// Each cell should only be touched once.
			_markers.set(_seed, _mark);

			// Prepare scanline marking. Capture the range for recursive calls.
			int count = 1;
			int rangeMin = _seed.x;
			int rangeMax = _seed.x + 1;

			// Go right and mark.
			GridCoord pos(_seed);
			while(true) {
				++pos.x;
				cell = _map.find(pos);
				if(!cell) break;
				if(!_isEmpty(*cell))
				{
					_markers.set(pos, _mark);
					++count;
					++rangeMax;
				} else break;
			}
			// Go left and mark.
			pos.x = _seed.x;
			while(true) {
				--pos.x;
				cell = _map.find(pos);
				if(!cell) break;
				if(!_isEmpty(*cell))
				{
					_markers.set(pos, _mark);
					++count;
					--rangeMin;
				} else break;
			}

			// The range consists of a line of connected cells -> anyting above or
			// below would be connected too. How these lines share neighbors depend on the grid type.
			int xOffUp = 0, xOffDown = 0;
			switch(MapT::TGridType)
			{
			case unsigned(GridType::QUAD_4): break; // Default values (rangeMin, rangeMax, xOffUp=0, xOffDown=0) are correct.
			case unsigned(GridType::QUAD_8): xOffUp = xOffDown = -1; --rangeMin; ++rangeMax; break;
			case unsigned(GridType::HEX): xOffUp = -1; ++rangeMax; break;
			}
			// Now, we can use a unified loop for any map type
			for(int i = rangeMin; i < rangeMax; ++i)
			{
				GridCoord posUp(i + xOffUp, _seed.y + 1);
				MarkerT * markerCell = _markers.find(posUp);
				if(!markerCell || *markerCell != _mark) count += scanlineFloodFill(_map, _markers, _mark, posUp, _isEmpty);
				GridCoord posDown(i + xOffDown, _seed.y - 1);
				markerCell = _markers.find(posDown);
				if(!markerCell || *markerCell != _mark) count += scanlineFloodFill(_map, _markers, _mark, posDown, _isEmpty);
			}

			return count;
		}
	}

	/// Clean all tiles, which are not connected with the seed.
	/// \returns the number of cells connected with seed (including seed)
	template<typename MapT>
	int extractComponent(MapT& _map, const GridCoord& _seed,
		std::function<bool(typename const MapT::TCellType&)> _isEmpty, std::function<void(typename MapT::TCellType&)> _clean)
	{
		// Create a temporary map for markers, with a sufficient size.
		Grid<MapT::TGridType, details::Marker> markerMap;
		// TODO: size?

		// Mark connected cells
		int count = details::scanlineFloodFill(_map, markerMap, details::Marker(true), _seed, _isEmpty);

		// Loop over all elements in map, clean all unmarked
		for(auto it = _map.begin(); it; ++it)
		{
			details::Marker * marker = markerMap.find(it.coord());
			if(!marker || !*marker) _clean(it.dat());
		}

		return count;
	}

	struct Component
	{
		GridCoord reprCoord;
		int size;
	};

	/// Find all connected tiles and mark them in a marker map.
	/// \param [out] _markerMap A map in which each connected component will be set to an individual
	///		number. The number corresponds to the index in the output. All empty tiles are set to -1.
	///		The map should be in a clean state without any number set.
	/// \returns A list of representative coordinates for each component (the first found tile of a
	///		component.
	template<typename MapT>// TODO _map const (iterator problems)
	std::vector<Component> findComponents(MapT& _map, Grid<MapT::TGridType, int>& _markerMap,
		std::function<bool(typename const MapT::TCellType&)> _isEmpty)
	{
		std::vector<Component> representatives;

		// Loop over all tiles and set to -1
		for(auto it = _map.begin(); it; ++it)
			_markerMap.set(it.coord(), -1);

		// Loop again and search the components
		for(auto it = _map.begin(); it; ++it)
		{
			if(!_isEmpty(it.dat()) && _markerMap.get(it.coord()) == -1)
			{
				int size = details::scanlineFloodFill(_map, _markerMap, int(representatives.size()), it.coord(), _isEmpty);
				representatives.push_back({it.coord(), size});
			}
		}

		return move(representatives);
	}

}} // namespace ca::map