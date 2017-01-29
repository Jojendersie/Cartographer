#pragma once

#include <ei/vector.hpp>
#include "ca/map/action/input.hpp"
#include <ca/pa/memory/refptr.hpp>

namespace ca { namespace map {
	
	/// A tile is the smallest logical unit on each map.
	/// It may have some graphical representation, an input action and unit triggers.
	class Tile : public ca::pa::ReferenceCountable
	{
	public:
		virtual ~Tile() = default;

		virtual void draw(const ei::Vec3& _position) const {}
		/// \param [in] _screenCoord An [0,1]^2 position on screen.
		virtual void processInput(const ei::Vec2& _screenCoord, MouseAction _mouseAction, float _mouseActAmount) {}
		
		/// \param [in] _neighbor A number in [0,8] for quad grids and [0,6] for hex grids.
		///		0 is the tile itself and the other numbers start at 1 for the north tile
		///		increasing in clock-wise direction.
		virtual void trigger(int _neighbor, class Unit* _unit) {}
		
		/// Solid objects block paths of units.
		virtual bool isSolid() const = 0;
	protected:
	};
	
}} // namespace ca::map
