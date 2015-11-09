#pragma once

#include <string>
#include "widget.hpp"
#include "properties/coordinate.hpp"

namespace cag {

	/// A standard clickable button with text and optional icon
	class Button: public Widget
	{
	public:
		Button();

		/// Set the button width and heigh (resets anchoring)
		void setSize(const Coord& _size);

		/// Set the position (resets anchoring)
		void setPosition(const Coord& _position);

		/// Set the button text (utf8 w/o BOM)
		/// \param [in] _text New text for the button (utf8 w/o BOM). The text is copied internally.
		void setText(const char* _text);
		const char* getText() { return m_text.c_str(); }

		/// Set an image as icon
		/// \param [in] _textureFile Name of an image file.
		/// \param [in] _side The size of the text on which the icon should be placed.
		///		if there is no text the icon is always centered.
		/// \param [in] _size Target size of the icon. If necessary the icon size is reduced
		///		automatically.
		/// \param [in] _padding Distance of the icon to the border and to the text.
		void setIcon(const char* _textureFile, SIDE _side, const Coord& _size, Range _padding = 2.0f);

		/// Set a custom look instead of the themes button rendering.
		/// \param [in] _textureFile Name of an image file.
		void setBackgroundTexture(const char* _textureFile);

		virtual void draw() override;
		/// Forward to Clickable::processInput
		virtual void processInput(const struct MouseState& _mouseState) override;
		/// Forward to Clickable::addOnButtonChangeFunc
		void addOnButtonChangeFunc(Clickable::OnButtonChange _callback, MouseState::ButtonState _stateMask) { m_clickComponent->addOnButtonChangeFunc(std::move(_callback), _stateMask); }
	private:
		std::string m_text;
		Coord m_textSize;	///< Precomputed size of the text for placement of the icon
		SIDE m_iconPos;
		Coord m_iconSize;
		Range m_iconPadding;
		uint64 m_iconTexture;
		uint64 m_backgroundTexture;
	};

} // namespace cag