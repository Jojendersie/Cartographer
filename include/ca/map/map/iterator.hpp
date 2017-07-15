#pragma once

// Generic iterators for any kind of grid implementation.
// These iterators supply a range of coordinates, which can then be accessed in any map.
// To use all of the iterators do the following:
// for(auto it = XXIterator(...); it; ++it)
//     access map.find(it.coord())...

#include "gridbase.hpp"

namespace ca { namespace map {

	/// \tparam GridT One of the members of enum class GridType.
	template<unsigned GridT>
	class NeighborIterator
	{
	};

	template<>
	class NeighborIterator<unsigned(GridType::QUAD_8)>
	{
	public:
		NeighborIterator(const GridCoord& _centerCoord, int _radius) :
			m_radius(_radius),
			m_dy(-_radius),
			m_dx(0),
			m_centerCoord(_centerCoord)
		{}

		// Pre increment
		NeighborIterator& operator ++ ()
		{
			++m_dx;
			if(m_dx > m_radius) { ++m_dy; m_dx = -m_radius; }
			return *this;
		}

		// Returns false for the end().
		operator bool () const { return m_dy <= m_radius; }

		GridCoord coord() const { return m_centerCoord + GridCoord(m_dx, m_dy); }
	private:
		const int m_radius; // Distance of iterated cells.
		int m_dx, m_dy;		// Position in Cube and Hex (z is implicit) grids.
		GridCoord m_centerCoord;
	};

	template<>
	class NeighborIterator<unsigned(GridType::HEX)>
	{
	public:
		NeighborIterator(const GridCoord& _centerCoord, int _radius) :
			m_radius(_radius),
			m_dy(-_radius),
			m_dx(0),
			m_centerCoord(_centerCoord)
		{}

		// Pre increment
		NeighborIterator& operator ++ ()
		{
			++m_dx;
			if(m_dx > ei::min(m_radius, -m_dy + m_radius)) {
				++m_dy;
				m_dx = ei::max(-m_radius, -m_dy - m_radius);
			}
			return *this;
		}

		// Returns false for the end().
		operator bool () const { return m_dy <= m_radius; }

		GridCoord coord() const { return m_centerCoord + GridCoord(m_dx, m_dy); }
	private:
		const int m_radius; // Distance of iterated cells.
		int m_dx, m_dy;		// Position in Cube and Hex (z is implicit) grids.
		GridCoord m_centerCoord;
	};

}} // namespace ca::map
