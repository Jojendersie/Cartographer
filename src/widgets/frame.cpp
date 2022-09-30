#include "ca/gui/widgets/frame.hpp"
#include "ca/gui/backend/mouse.hpp"
#include "ca/gui/backend/renderbackend.hpp"
#include "ca/gui/guimanager.hpp"
#include "ca/gui/rendering/theme.hpp"

namespace ca { namespace gui {

	Frame::Frame() :
		Resizeable(this),
		Moveable(this),
		m_opacity(1.0f),
		m_passive(false),
		m_texture(0),
		m_color{-1.0f}
	{
		m_autoResize = false; // Overwrite group policy
		setMoveable(false);
		setResizeable(false);
	}

	Frame::~Frame()
	{
	}

	void Frame::draw() const
	{
		if(m_visible)
		{
			// Set clipping region for this and all subelements
			bool vis = GUIManager::pushClipRegion(rectangle());
			if(vis)
			{
				// Draw the frame background
				if(m_texture)
					GUIManager::theme().drawImage(rectangle(), m_texture, m_opacity, m_tiling);
				else
					GUIManager::theme().drawBackgroundArea(rectangle(), m_opacity, m_color);

				// Draw all contained children of the group
				Group::draw();
			}

			GUIManager::popClipRegion();
		}
	}

	bool Frame::processInput(const MouseState& _mouseState)
	{
		// Component disabled?
		if(!isEnabled() || !isVisible()) return false;
		if(Group::processInput(_mouseState)) return true;
		// Input was not consumed by an element.
		// If there are properties try them. Since move and resize require input handling outside
		// the reference frame this is not inside the isMouseOver-block.
		if(m_passive) return false;
		return Widget::processInput(_mouseState);
	}

	void Frame::setBackground(const char* _imageFile, bool _smooth, float _opacity, bool _tiling)
	{
		m_texture = GUIManager::renderBackend().getTexture(_imageFile, _smooth);
		m_opacity = _opacity;
		m_tiling = _tiling;
	}

	void Frame::setBackgroundOpacity(float _opacity)
	{
		m_opacity = _opacity;
	}


}} // namespace ca::gui