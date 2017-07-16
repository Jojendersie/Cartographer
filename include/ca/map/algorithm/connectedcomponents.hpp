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
		template<typename MapT>
		int scanlineFloodFill(const MapT& _map, Grid<MapT::TGridType, Marker>& _markers, const GridCoord& _seed, const std::function<bool(typename const MapT::TCellType&)>& _isEmpty)
		{
			auto * cell = _map.find(_seed);
			if(!cell) return 0;
			// Each cell should only be touched once.
			_markers.set(_seed, true);
			if(_isEmpty(*cell)) return 0;


			// Prepare scanline marking. Capture the range for recursive calls.
			int count = 1;
			int rangeMin = _seed.x;
			int rangeMax = _seed.x + 1;

			// Go right and mark.
			GridCoord pos(_seed);
			while(true) {
				++pos.x;
				cell = _map.find(pos);
				if(cell) _markers.set(pos, true); else break;
				if(!_isEmpty(*cell))
				{
					_markers.set(pos, true);
					++count;
					++rangeMax;
				} else break;
			}
			// Go left and mark.
			pos.x = _seed.x;
			while(true) {
				--pos.x;
				cell = _map.find(pos);
				if(cell) _markers.set(pos, true); else break;
				if(!_isEmpty(*cell))
				{
					_markers.set(pos, true);
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
				details::Marker * markerCell = _markers.find(posUp);
				if(!markerCell || !*markerCell) count += scanlineFloodFill(_map, _markers, posUp, _isEmpty);
				GridCoord posDown(i + xOffDown, _seed.y - 1);
				markerCell = _markers.find(posDown);
				if(!markerCell || !*markerCell) count += scanlineFloodFill(_map, _markers, posDown, _isEmpty);
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
		int count = details::scanlineFloodFill(_map, markerMap, _seed, _isEmpty);

		// Loop over all elements in map, clean all unmarked
		for(auto it = _map.begin(); it; ++it)
		{
			details::Marker * marker = markerMap.find(it.coord());
			if(!marker || !*marker) _clean(it.dat());
		}

		return count;
	}

}} // namespace ca::map