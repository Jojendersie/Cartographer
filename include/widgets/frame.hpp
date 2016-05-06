#pragma once

#include <vector>
#include "widget.hpp"
#include "group.hpp"

namespace ca { namespace gui {

	/// The frame is a container with a drawn background and clipping for its subcomponents.
	class Frame : public Group
	{
	public:
		Frame(bool _anchorable, bool _clickable, bool _moveable, bool _resizeable);
		~Frame();

		/// Implement the draw method
		void draw() override;

		/// Forward input to subelements and to properties
		virtual bool processInput(const struct MouseState& _mouseState) override;

		/// Check if the given child is the first one
//		virtual bool isChildFocused(const Widget* _child) const override;

		/// Set an image as background. It is stretched automatically.
		/// \param [in] _imageFile Name of a texture file loadable by the render-backend.
		/// \param [in] _smooth Interpolate linearly during texture sampling or use nearest point.
		/// \param [in] _opacity An [0,1] transparency value (alpha). The value is multiplied with
		///		the image during rendering. The default alpha is opaque (1).
		void setBackground(const char* _imageFile, bool _smooth = true, float _opacity = 1.0f);
		// TODO: recursive transparency?
		void setBackgroundOpacity(float _opacity);

	private:
		float m_opacity;
		uint64 m_texture;
	};

	typedef std::shared_ptr<Frame> FramePtr;

}} // namespace ca::gui