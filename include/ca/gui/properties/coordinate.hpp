#pragma once

#include <ei/vector.hpp>

namespace ca { namespace gui {

	// Internal coordinate format: pixels with fractional parts
	typedef ei::Vec2 Coord2;
	typedef float Coord;

	namespace coord {

		/// Convert [0,1]^2 screen coordinate to internal
		Coord2 percentage(float _x, float _y);
		Coord percentageWidth(float _w);
		Coord percentageHeight(float _h);
		/// Convert a [0,a]x[0,1] height-relative coordinate to internal, where 'a' is the aspect
		/// ratio w/h.
		/// \details This coordinate allows to design a resolution independent GUI with
		///		proportional width and height properties. I.e. if using _x==_y for the size then
		///		the element will be square.
		Coord2 relative(float _x, float _y);
		Coord relative(float _x);
		/// Return the maximum x-value of the height-relative [0,a]x[0,1] coordinate system.
		/// This is the same as the aspect ration w/h.
		float maxRelativeWidth();
		/// Convert a [0,a]x[0,1] height-relative coordinate to internal with rounding to integral
		/// pixels.
		Coord2 relativeI(float _x, float _y);
		Coord relativeI(float _x);
		/// Convert an absolute pixel coordinate to internal
		Coord2 pixel(int _x, int _y);
		Coord pixel(int _pixels);

	} // namespace coord
}} // namespace ca::gui