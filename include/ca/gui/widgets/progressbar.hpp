#pragma once

#include <string>
#include "widget.hpp"
#include "ca/gui/properties/textproperties.hpp"

namespace ca { namespace gui {

	/// A progress bar with or without images and labels.
	// TODO: draw color or gradient stuff
	class ProgressBar : public Widget, public TextProperties
	{
	public:
		ProgressBar();

		/// Implement the draw method
		void draw() const override;

		/// Set the image which is shown over the entire component.
		/// \param [in] _backgroundTextureFile Name of an image file. This image is
		///		always drawn in full extend.
		/// \param [in] _foregroundTextureFile Name of an image file. This images is
		///		shown partially dependent on the progress.
		void setImages(const char* _backgroundTextureFile, const char* _foregroundTextureFile, bool _smooth = true);

		/// Optional text which is shown in the center (utf8 w/o bom)
		void setText(const char* _text);
		const char* getText() { return m_text.c_str(); }

		/// If the foreground content covers the other one only partially the margins
		/// define where the real progress starts and ends. The images should always have
		/// the same size anyway.
		/// In horizontal mode only left and right are used and in vertical the other two.
		void setMargin(Coord _left, Coord _right, Coord _bottom, Coord _top);

		/// \param [in] _progress A number in [0,1] for the progress. Numbers outside
		///		the range will be clipped.
		void setProgress(float _progress);
		float getProgress() const { return m_progress; }

		void setProgressDirection(bool _vertical) { m_vertical = _vertical; }
	private:
		uint64 m_backgroundTexture;
		uint64 m_foregroundTexture;
		float m_progress;
		bool m_vertical;
		Coord m_marginLeft, m_marginRight, m_marginBottom, m_marginTop;
		std::string m_text;
		ei::Vec2 m_textSize;
	};

	typedef pa::RefPtr<ProgressBar> ProgressBarPtr;

}} // namespace ca::gui
