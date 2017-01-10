#pragma once

#include <string>
#include "ca/gui/widgets/widget.hpp"
#include "ca/gui/properties/coordinate.hpp"
#include "ca/gui/properties/textproperties.hpp"

namespace ca { namespace gui {

	/// A standard clickable button with text and optional icon
	class Button : public Widget, public TextProperties
	{
	public:
		Button();

		/// Implement the draw method
		void draw() const override;

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
		/// \param [in] _smooth Interpolate linearly during texture sampling or use nearest point.
		/// \param [in] _padding Distance of the icon to the border and to the text.
		void setIcon(const char* _textureFile, SIDE::Val _side, const Coord2& _size, bool _smooth = true, Coord _padding = 2.0f);

		/// Set a custom look instead of the theme's background rendering.
		/// \param [in] _textureFile Name of an image file.
		/// \param [in] _smooth Interpolate linearly during texture sampling or use nearest point.
		void setBackgroundTexture(const char* _textureFile, bool _smooth = true);

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

	typedef pa::RefPtr<Button> ButtonPtr;

}} // namespace ca::gui