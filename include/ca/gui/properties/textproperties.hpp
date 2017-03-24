#pragma once

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

		// TODO: Color properties?
	protected:
		float m_relativeTextSize;

		TextProperties() :
			m_relativeTextSize(1.0f)
		{}

		// Optional method to react on changes in the text properties
		virtual void onTextScaleChanged() {};
	};

}} // namespace ca::gui