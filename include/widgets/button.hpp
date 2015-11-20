#pragma once

#include <string>
#include "widget.hpp"
#include "properties/coordinate.hpp"

namespace ca { namespace gui {

	/// A standard clickable button with text and optional icon
	class Button : public Widget
	{
	public:
		Button();

		/// Implement the draw method
		void draw() override;

		/// Set the button width and heigh (resets anchoring)
		void setSize(const Coord2& _size);

		/// Set the position (resets anchoring)
		void setPosition(const Coord2& _position);

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
		void setIcon(const char* _textureFile, SIDE::Val _side, const Coord2& _size, Coord _padding = 2.0f);

		/// Set a custom look instead of the themes button rendering.
		/// \param [in] _textureFile Name of an image file.
		void setBackgroundTexture(const char* _textureFile);

		/// Forward to Clickable::processInput
		//virtual bool processInput(const struct MouseState& _mouseState) override;
		/// Forward to Clickable::addOnButtonChangeFunc
		void addOnButtonChangeFunc(Clickable::OnButtonChange _callback, MouseState::ButtonState _stateMask) { m_clickComponent->addOnButtonChangeFunc(std::move(_callback), _stateMask); }
	private:
		std::string m_text;
		Coord2 m_textSize;	///< Precomputed size of the text for placement of the icon
		SIDE::Val m_iconPos;
		Coord2 m_iconSize;
		Coord m_iconPadding;
		uint64 m_iconTexture;
		uint64 m_backgroundTexture;
	};

}} // namespace ca::gui