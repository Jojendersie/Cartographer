#include "ca/gui/widgets/image.hpp"
#include "ca/gui/guimanager.hpp"
#include "ca/gui/rendering/theme.hpp"
#include "ca/gui/backend/renderbackend.hpp"

namespace ca { namespace gui {

	Image::Image() :
		m_imageTexture(0)
	{
	}

	void Image::draw() const
	{
		if(m_imageTexture)
			GUIManager::theme().drawImage(rectangle(), m_imageTexture);
	}

	void Image::setTexture(const char* _textureFile, bool _smooth)
	{
		m_imageTexture = GUIManager::renderBackend().getTexture(_textureFile, _smooth);
	}



	ImageButton::ImageButton() :
		Clickable(this)
	{
	}

}} // namespace ca::gui