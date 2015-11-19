#include "properties/coordinate.hpp"
#include "cagui.hpp"

namespace cag {

	namespace coord {

		Coord percentage(float _x, float _y)
		{
			return ei::Vec2(_x * GUIManager::getWidth(), _y * GUIManager::getHeight());
		}

		Range percentageWidth(float _w)
		{
			return _w * GUIManager::getWidth();
		}

		Range percentageHeight(float _h)
		{
			return _h * GUIManager::getHeight();
		}

		Coord relative(float _x, float _y)
		{
			return ei::Vec2(_x * GUIManager::getHeight(), _y * GUIManager::getHeight());
		}

		Range relativeWidth(float _w)
		{
			return _w * GUIManager::getHeight();
		}

		Range relativeHeight(float _h)
		{
			return _h * GUIManager::getHeight();
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