#pragma once

#include <vector>
#include "widget.hpp"

namespace ca { namespace gui {

	/// The group is the simplest form of an element container.
	///
	/// The group only forwards everything without thinking. It does not render itself,
	/// makes no clipping and is never resized.
	/// TODO: positioning might make sense.
	/// You can show/hide or enable/disable all contained elements by
	///	show/hide/enable/disable all the group.
	class Group : public Widget
	{
	public:
		Group();

		/// Implement the draw method
		void draw() const override;

		/// Add a new child element.
		/// \param [in] _innerLayer Stir rendering order within the component.
		///		The larger the layer the later is the element drawn (more in foreground).
		void add(WidgetPtr _widget, unsigned _innerLayer = 0);

		/// Find and remove a widget (O(n) with n = number of elements)
		void remove(WidgetPtr _widget);

		/// Forward input to subelements and to properties
		virtual bool processInput(const struct MouseState& _mouseState) override;

		/// Recursive refit
		virtual void refitToAnchors() override;
	protected:
		struct WEntry {
			WidgetPtr widget;
			unsigned innerLayer;
		};
		std::vector<WEntry> m_children;	///< List of subelements. The list is sorted after layers and last focus time (last element inside a layer has the focus).

	private:
		/// Resort the children list to bring focused element to the front.
		void moveToFront(size_t _index);
	};

	typedef pa::RefPtr<Group> GroupPtr;

}} // namespace ca::gui