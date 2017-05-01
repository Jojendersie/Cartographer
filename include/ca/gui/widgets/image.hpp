#pragma once

#include "widget.hpp"

namespace ca { namespace gui {

	/// Simple image element
	class Image : public Widget
	{
	public:
		Image();

		/// Implement the draw method
		void draw() const override;

		/// Set the image which is shown over the entire component.
		/// \param [in] _textureFile Name of an image file.
		void setTexture(const char* _textureFile, bool _smooth = true);

	private:
		uint64 m_imageTexture;
	};

	typedef pa::RefPtr<Image> ImagePtr;

}} // namespace ca::gui
