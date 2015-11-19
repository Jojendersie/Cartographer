#include "properties/coordinate.hpp"
#include "cagui.hpp"

namespace cag {

	namespace coord {

		Coord2 percentage(float _x, float _y)
		{
			return ei::Vec2(_x * GUIManager::getWidth(), _y * GUIManager::getHeight());
		}

		Coord percentageWidth(float _w)
		{
			return _w * GUIManager::getWidth();
		}

		Coord percentageHeight(float _h)
		{
			return _h * GUIManager::getHeight();
		}

		Coord2 relative(float _x, float _y)
		{
			return ei::Vec2(_x * GUIManager::getHeight(), _y * GUIManager::getHeight());
		}

		Coord relativeWidth(float _w)
		{
			return _w * GUIManager::getHeight();
		}

		Coord relativeHeight(float _h)
		{
			return _h * GUIManager::getHeight();
		}

		Coord2 pixel(int _x, int _y)
		{
			return ei::Vec2((float)_x, (float)_y);
		}

		Coord pixel(int _pixels)
		{
			return (float)_pixels;
		}

	} // namespace coord

} // namespace cag