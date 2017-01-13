#pragma once

#include "ca/map/map/grid.hpp"

namespace ca { namespace map {
	
	/// A construct is a fixed structure which covers multiple tiles.
	class Construct
	{
	public:
		/// Return a mask which tiles are solid.
		/// \details The meaning of the ints can be specified manually.
		///		It should be used in Grid::compare actions.
		virtual Grid<int> getOccupancy() const = 0;
	};
}}