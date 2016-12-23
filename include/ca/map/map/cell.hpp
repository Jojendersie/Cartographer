#pragma once

#include "ca/map/map/tile.hpp"

namespace ca { namespace map {
	
	/// A cell is a structuring unit which may contain one tile per layer
	template<uint N>
	class Cell
	{
	public:
		/// Get a tile from a certain layer. Might be 0 if not occupied.
		Tile* getTile(uint _layer) { return m_layer[_layer].get(); }
		const Tile* getTile(uint _layer) const { return m_layer[_layer].get(); }
		
		/// Set or replace a tile
		void setTile(uint _layer, std::unique_ptr<Tile> _tile) {
			m_layer[_layer] = move(_tile);
		}
		
		/// Is a certain layer in this cell counting as occupied?
		bool isOccupied(uint _layer) const
		{
			return (m_layer[_layer] != nullptr) && m_layer[_layer]->isOccupied();
		}
		
		/// Is any layer in this cell counting as occupied?
		bool isOccupied() const
		{
			for(uint i = 0; i < N; ++i)
				if( (m_layer[_layer] != nullptr) && m_layer[_layer]->isOccupied() )
					return true;
			return false;
		}
		
		/// Does any layer has solid occupancy?
		bool isSolid() const
		{
			for(uint i = 0; i < N; ++i)
				if( (m_layer[_layer] != nullptr) && m_layer[_layer]->isOccupied() )
					return true;
			return false;
		}
	private:
		std::unique_ptr<Tile> m_layer[N];
	};
	
}} // namespace ca::map
	