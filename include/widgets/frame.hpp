#pragma once

#include <vector>
#include "widget.hpp"

namespace cag {

	/// The frame is the simplest form of a element container.
	class Frame: public Widget
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
	protected:
		std::vector<WidgetPtr> m_activeChildren;	///< List of subelements which can receive input. The list is sorted after last focus time (first element has the focus)
		std::vector<WidgetPtr> m_passiveChildren;	///< List of subelements which are only drawn

	private:
		/// Resort active list to bring focussed element to the front.
		void focusOn(size_t _index);
	};

} // namespace cag