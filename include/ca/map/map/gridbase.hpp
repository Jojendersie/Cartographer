#pragma once

#include <ei/vector.hpp>

namespace ca { namespace map {

	enum class GridType {
		QUAD_4 = 4,		// A quad grid with 4-neighbors
		QUAD_8 = 8,		// A quad grid with 8-neighbors
		// About hex-grid coordinates have a look on http://www.redblobgames.com/grids/hexagons/.
		// There is a trick using cube coordinates and restricting them to the diagonal slicing
		// plane x + y + z = 0.
		// This allows easy distance computations, adding coordinates...
		// To get an coordinate for memory accesses cube coordinates can be converted into axial
		// coordinates by projection (e.g. discard z). Thus, y gives the row index and x the
		// column index.
		HEX = 6,
	};

	typedef ei::IVec2 GridCoord;
	typedef ei::Vec2 GridCoordF;
	typedef ei::Vec2 GridPos;

	/// Round a floating point coordinate into a grid coordinate.
	template<unsigned GridT>
	inline GridCoord roundCoordinate(const GridPos& _p)
	{
		return round(_p);
	}

	/// Special round method to convert floating point hex-coordinates into
	/// a grid hex-coordinate.
	template<>
	inline GridCoord roundCoordinate<unsigned(GridType::HEX)>(const GridPos& _p)
	{
		GridPos p = GridPos(_p.x * 1.732050808f - _p.y, _p.y * 2.0f) / 3.0f;
		// Convert Axial to Cube (x+y+z=0) coordinates and round
		int ix = ei::round(p.x);
		int iy = ei::round(-p.x - p.y);
		int iz = ei::round(p.y);

		// Check if fractions satisfy x+y+z=0 constraint
		float fx = abs(ix - p.x);
		float fy = abs(iy + p.x + p.y);
		float fz = abs(iz - p.y);

		if(fx > fy && fx > fz)
			ix = -iy-iz;
		else if(fz > fy)
			iz = -ix-iy;
		// Neglegt case where y is largest because we discard it anyway.

		return GridCoord(ix, iz);
	}

	/// Convert a floating position into a grid coordinate.
	/// \details Simply casts in QUAD grids.
	template<unsigned GridT>
	inline GridPos realPosition(const GridCoord& _coord)
	{
		return GridPos(_coord);
	}
	template<unsigned GridT>
	inline GridPos realPosition(const GridCoordF& _coord)
	{
		return _coord;
	}

	/// \details HEX grids have alternating x-coords.
	template<>
	inline GridPos realPosition<unsigned(GridType::HEX)>(const GridCoord& _coord)
	{
		return GridPos(1.732050808f * (_coord.x + _coord.y * 0.5f), 1.5f * _coord.y);
	}
	template<>
	inline GridPos realPosition<unsigned(GridType::HEX)>(const GridCoordF& _coord)
	{
		return GridPos(1.732050808f * (_coord.x + _coord.y * 0.5f), 1.5f * _coord.y);
	}

	/// Determines the minimal number of grid cells between two locations.
	/// This is the Manhatten-distance for quad-4 grids and something similar
	/// for hex grids.
	template<unsigned GridT>
	inline int distance(const GridCoord& _a, const GridCoord& _b)
	{
		return abs(_b.x - _a.x) + abs(_b.y - _a.y);
	}
	template<>
	inline int distance<unsigned(GridType::QUAD_8)>(const GridCoord& _a, const GridCoord& _b)
	{
		return max(abs(_b - _a));
	}
	template<>
	inline int distance<unsigned(GridType::HEX)>(const GridCoord& _a, const GridCoord& _b)
	{
		return (abs(_a.x - _b.x) + abs(_a.x + _a.y - _b.x - _b.y) + abs(_a.y - _b.y)) / 2;
	}

	// Shorter names avoiding the lengthy casted-enum to specialize manually.
	// Unfortunatelly, there is no using... The cleanest way would be a variadic
	// with perfect forwarding.
#define roundCoordQuad	roundCoordinate<unsigned(ca::map::GridType::QUAD_4)>
#define roundCoordHex	roundCoordinate<unsigned(ca::map::GridType::HEX)>
#define realPosQuad		realPosition<unsigned(ca::map::GridType::QUAD_4)>
#define realPosHex		realPosition<unsigned(ca::map::GridType::HEX)>
#define	distanceQuad4	distance<unsigned(ca::map::GridType::QUAD_4)>
#define	distanceQuad8	distance<unsigned(ca::map::GridType::QUAD_8)>
#define	distanceHex		distance<unsigned(ca::map::GridType::HEX)>

}} // namespace ca::map