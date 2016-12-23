#pragma once

namespace ca { namespace map {
	
	/// A construct is a fixed structure which covers multiple tiles.
	class Construct
	{
	public:
		/// Return a mask which tiles are solid.
		virtual Grid<Occupancy> getOccupancy() const = 0;
	};
}}