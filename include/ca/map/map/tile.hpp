#pragma once

#include "ca/action/input.hpp"

namespace ca { namespace map {
	
	enum class Occupancy
	{
		AMBIENT,	// Counts like unoccupyed cells
		OCCUPIED,	// Occupied but still pass able
		SOLID,		// Non pass able
	};
	
	/// A tile is the smallest logical unit on each map.
	/// It may have some graphical representation, an input action and unit triggers.
	class Tile
	{
	public:
		virtual void draw() const {}
		/// \param [in] _screenCoord An [0,1]^2 position on screen.
		virtual void processInput(const ei::Vec2& _screenCoord, MouseAction _mouseAction, float _mouseActAmount) {}
		
		/// \param [in] _neighbor A number in [0,8] for quad grids and [0,6] for hex grids.
		///		0 is the tile itself and the other numbers start at 1 for the north tile
		///		increasing in clock-wise direction.
		virtual void trigger(int _neighbor, Unit* _unit) {}
		
		/// Solid objects block paths of units.
		bool isSolid() const { return m_occupancy == Occupancy::SOLID; }
		bool isOccupied() const { return (m_occupancy == Occupancy::SOLID) || (m_occupancy == Occupancy::OCCUPIED); }
	protected:
		Occupancy m_occupancy;	///< Each tile should be occupied or solid. None is reserved for empty cells.
	};
	
}} // namespace ca::map
