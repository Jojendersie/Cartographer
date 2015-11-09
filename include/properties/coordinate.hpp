#pragma once

#include <ei/vector.hpp>

namespace cag {

	// Internal coordinate format: pixels
	typedef ei::Vec2 Coord;
	typedef float Range;

	namespace coord {

		/// Convert [0,1]^2 GUI coordinate to internal
		Coord percentage(float _x, float _y);
		Range percentageWidth(float _w);
		Range percentageHeight(float _h);
		/// Convert a [0,a]x[0,1] height-relative coordinate to internal, where a is the aspect
		/// ratio w/h.
		/// \details This coordinate allows to design a resolution independent GUI with
		///		proportional width and height properties. I.e. if using _x==_y for the size then
		///		the element will be square.
		Coord relative(float _x, float _y);
		Range relativeWidth(float _w);
		Range relativeHeight(float _h);
		/// Convert an absolute pixel coordinate to internal
		Coord pixel(int _x, int _y);
		Range pixel(int _pixels);

	} // namespace coord
} // namespace cag