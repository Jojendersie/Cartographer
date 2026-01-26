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

		void onExtentChanged() override;

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
		void setBackgroundColor(const ei::Vec4& _color) { m_color = ei::Vec3{_color}; m_opacity = _color.a; }
		ei::Vec4 getBackgroundColor() const { return ei::Vec4{m_color, m_opacity}; }

		// A passive frame lets inputs to widgets below the frame through. It behaves similar to
		// a disabled frame, except that widgets inside the frame receive their inputs as usual.
		void setPassive(bool _passive) { m_passive = _passive; }
		bool isPassive() const { return m_passive; }

		/// Enable automatic scrolling of the content.
		/// _horizontal: Enables horizontal scrolling, adding a scrollbar at the bottom.
		/// _vertical: Enables vertical scrolling, adding a scrollbar at the right.
		/// _margin: Additional space around all child compents the extends the total visible area.
		/// _scrollbarWidth: Width of the scrollbars in pixels.
		void setScrolling(const bool _horizontal, const bool _vertical, const float _margin, const Coord _scrollbarWidth);
		bool isScrollingEnabled() const;
	private:
		float m_opacity;
		bool m_tiling;
		bool m_passive;
		uint64 m_texture;
		ei::Vec3 m_color;

		// Scrolling related properties
		class ScrollBar* m_horizontalScrollbar;
		class ScrollBar* m_verticalScrollbar;
		float m_scrollMargin;
		// Before any interaction happens we should make sure the automatic scrollbars are up to date.
		// Using an update flag avoid quadratic overhead when many elements are added in a row.
		bool m_scrollbarContentSizeDirty = false;

		// Computes the content bounding box if scrolling is enabled.
		void resetScrollbarContentSize();
		friend void childChangedCallback(Widget* _this, Widget* _child, bool _add);
		friend void onScrollChangedCallback(Widget* _this, const float _amount);
	};

	typedef pa::RefPtr<Frame> FramePtr;

}} // namespace ca::gui