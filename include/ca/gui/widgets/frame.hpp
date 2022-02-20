#pragma once

#include <vector>
#include "widget.hpp"
#include "group.hpp"

namespace ca { namespace gui {

	/// The frame is a container with a drawn background and clipping for its subcomponents.
	class Frame : public Group, public Resizeable, public Moveable
	{
	public:
		Frame();
		~Frame();

		/// Implement the draw method
		void draw() const override;

		/// Forward input to subelements and to properties
		virtual bool processInput(const struct MouseState& _mouseState) override;

		/// Check if the given child is the first one
//		virtual bool isChildFocused(const Widget* _child) const override;

		/// Set an image as background. It is stretched automatically.
		/// \param [in] _imageFile Name of a texture file loadable by the render-backend.
		/// \param [in] _smooth Interpolate linearly during texture sampling or use nearest point.
		/// \param [in] _opacity An [0,1] transparency value (alpha). The value is multiplied with
		///		the image during rendering. The default alpha is opaque (1).
		/// \param [in] _tiling Repeat the texture (the texture is scaled pixel perfect).
		void setBackground(const char* _imageFile, bool _smooth = true, float _opacity = 1.0f, bool _tiling = false);
		// TODO: recursive transparency?
		void setBackgroundOpacity(float _opacity);
		float getBackgroundOpacity() const { return m_opacity; }

		// A passive frame lets inputs to widgets below the frame through. It behaves similar to
		// a disabled frame, except that widgets inside the frame receive their inputs as usual.
		void setPassive(bool _passive) { m_passive = _passive; }
		bool isPassive() const { return m_passive; }
	private:
		float m_opacity;
		bool m_tiling;
		bool m_passive;
		uint64 m_texture;
	};

	typedef pa::RefPtr<Frame> FramePtr;

}} // namespace ca::gui