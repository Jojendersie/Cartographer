#pragma once

#include <ei/vector.hpp>

namespace ca { namespace gui {

	/// Interface addon for many text based components. Can change
	/// properties relative to the current
	class TextProperties
	{
	public:
		/// Change text size relative to the theme property.
		/// \param [in] _relativeSize Scale factor. E.g. 0.5 will half the size and
		///		3.0 tripple it.
		void setTextScale(float _relativeSize) { m_relativeTextSize = _relativeSize; onTextScaleChanged(); }
		float getTextScale() const { return m_relativeTextSize; }

		/// Set a custom color for a text. The initial color depends on the theme.
		void setTextColor(const ei::Vec4& _color) { m_textColor = _color; }
		const ei::Vec4& getTextColor() const { return m_textColor; }
	protected:
		float m_relativeTextSize;
		ei::Vec4 m_textColor;

		TextProperties() :
			m_relativeTextSize(1.0f),
			m_textColor(-1.0f) // Use theme default
		{}

		// Optional method to react on changes in the text properties
		virtual void onTextScaleChanged() {};
	};

}} // namespace ca::gui