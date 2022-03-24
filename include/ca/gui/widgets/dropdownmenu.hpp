#pragma once

#include "widget.hpp"
#include "scrollbar.hpp"

namespace ca { namespace gui {

	class DropDownListIntenral;

	// A list of widgets that is extended/hidden upon a click. Used to make choices.
	// The items in the list can have a text, an icon and a describing popup.
	// Items are indexed top to bottom.
	class DropDownMenu : public Widget
	{
	public:
		DropDownMenu();

		void setMaxShownItems(int _num) { m_maxNumShown = _num; }
		int getMaxShownItems() const { return m_maxNumShown; }

		// Adds an item at the end of the list and returns its index.
		int addItem(const char* _text, const char* _textureFile = nullptr, bool _smooth = true);

		// Delete the item at an index.
		void removeItem(int _idx);

		int getNumItems() const;

		const char* getItemText(int _idx) const;
		void setItemText(int _idx, const char* _text);

		const uint64 getItemIcon(int _idx) const;
		void setItemIcon(int _idx, const char* _textureFile, bool _smooth);

		// Get distance between boundary and icon as well as between (largest) icon and text.
		//  margin
		//   /   \
		// |   ##   Item with larger icon
		// |   #_   Item with icon
		// |   __   Item without icon
		// _ == additional automatically determined space for the largest icon.
		// 
		// Icon sizes are determined relative to the ItemHeight-2*margin. If the item
		// is smaller than this value it will be vertically centered without scaling.
		// Otherwise, it will be scaled down.
		// I.e. the margin is also applied vertically between item boundary and icon.
		Coord getItemTextMargin() const;
		// Set the distances between icon text and boundary. (see getItemTextMargin())
		void setItemTextMargin(Coord _margin);

		const Widget* getItemInfoPopup(int _idx) const;
		Widget* getItemInfoPopup(int _idx);
		void setItemInfoPopup(int _idx, WidgetPtr _popup);

		/// Get the index of the selected item. -1 if empty.
		int getSelected() const { return m_selected; }
		void setSelected(int _idx);

		/// Implement the draw method
		void draw() const override;

		/// Process mouse input for open/close selection and scrolling.
		/// \param [in] _mouseState State of the mouse buttons and position. Required to start/end
		///		movements.
		bool processInput(const struct MouseState & _mouseState) override;

		/// Type for selection callbacks.
		/// \param [in] _thisBox A pointer to the widget for which the event is triggered.
		/// \param [in] _newValue The new value. The old one is consequently !_newValue.
		typedef std::function<void(class DropDownMenu* _this, const int _newValue)> OnSelect;

		/// Replace the onSelect function. The function will be called whenever the value of
		/// the selection changed.
		void setOnSelect(OnSelect _callback);
	private:
		ScrollBar m_bar;					//< A scroll bar if there are more items than can be shown.
		pa::RefPtr<DropDownListIntenral> m_list;	//< A subwidget which can be shown on top of other things, that contains the actual graphical reperesentation of the list.
		OnSelect m_onSelect;				//< Callback for changes.
		int m_maxNumShown;					//< Number of shown items. If there are more, they need to be scrolled.
		int m_selected;						//< The index of the selected item or -1.
		Coord m_itemTextMargin;				//< Space from the left to the start of the text.

	protected:
		virtual void onExtentChanged(bool _positionChanged, bool _sizeChanged) override;
	};

	typedef pa::RefPtr<DropDownMenu> DropDownMenuPtr;
	typedef pa::RefPtr<const DropDownMenu> ConstDropDownMenuPtr;

}} // namespace ca::gui
