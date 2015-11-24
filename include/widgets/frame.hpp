#pragma once

#include <vector>
#include "widget.hpp"

namespace ca { namespace gui {

	/// The frame is the simplest form of a element container.
	class Frame : public Widget
	{
	public:
		Frame(bool _anchorable, bool _clickable, bool _moveable, bool _resizeable);

		/// Implement the draw method
		void draw() override;

		/// Add a new child element.
		void add(WidgetPtr _widget);

		/// Find and remove a widget (O(n) with n = number of elements)
		void remove(WidgetPtr _widget);

		/// Forward input to subelements and to properties
		virtual bool processInput(const struct MouseState& _mouseState) override;

		/// Check if the given child is the first one
		virtual bool isChildFocused(const Widget* _child) const override;

		/// Set an image as background. It is streched automatically.
		/// \param [in] _imageFile Name of a texture file loadable by the render-backend.
		/// \param [in] _smooth Interpolate linearly during texture sampling or use nearest point.
		/// \param [in] _opacity An [0,1] transparents value (alpha). The value is multiplied with
		///		the image during rendering. The default alpha is opaque (1).
		void setBackground(const char* _imageFile, bool _smooth = true, float _opacity = 1.0f);
		// TODO: recursive transparency?
		void setBackgroundOpacity(float _opacity);
	protected:
		std::vector<WidgetPtr> m_activeChildren;	///< List of subelements which can receive input. The list is sorted after last focus time (first element has the focus)
		std::vector<WidgetPtr> m_passiveChildren;	///< List of subelements which are only drawn

	private:
		/// Resort active list to bring focussed element to the front.
		void focusOn(size_t _index);

		float m_opacity;
		uint64 m_texture;
	};

	typedef std::shared_ptr<Frame> FramePtr;

}} // namespace ca::gui