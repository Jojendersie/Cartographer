#include "properties/coordinate.hpp"
#include "cagui.hpp"

namespace cag {

	namespace coord {

		Coord percentage(float _x, float _y)
		{
			return ei::Vec2(_x * GUIManagar::getWidth(), _y * GUIManagar::getHeight());
		}

		Range percentageWidth(float _w)
		{
			return _w * GUIManagar::getWidth();
		}

		Range percentageHeight(float _h)
		{
			return _h * GUIManagar::getHeight();
		}

		Coord relative(float _x, float _y)
		{
			return ei::Vec2(_x * GUIManagar::getHeight(), _y * GUIManagar::getHeight());
		}

		Range relativeWidth(float _w)
		{
			return _w * GUIManagar::getHeight();
		}

		Range relativeHeight(float _h)
		{
			return _h * GUIManagar::getHeight();
		}

		Coord pixel(int _x, int _y)
		{
			return ei::Vec2((float)_x, (float)_y);
		}

		Range pixel(int _pixels)
		{
			return (float)_pixels;
		}

	} // namespace coord

} // namespace cag