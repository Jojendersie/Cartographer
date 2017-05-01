#include "ca/gui/properties/coordinate.hpp"
#include "ca/gui.hpp"

namespace ca { namespace gui {

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

		Coord relative(float _x)
		{
			return _x * GUIManager::getHeight();
		}

		float maxRelativeWidth()
		{
			return GUIManager::getWidth() / float(GUIManager::getHeight());
		}

		Coord2 relativeI(float _x, float _y)
		{
			return ei::Vec2(round(_x * GUIManager::getHeight()), round(_y * GUIManager::getHeight()));
		}

		Coord relativeI(float _x)
		{
			return round(_x * GUIManager::getHeight());
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

}} // namespace ca::gui
