#pragma once

#include <memory>
#include "ca/gui/properties/anchorable.hpp"
#include "ca/gui/properties/anchorprovider.hpp"
#include "ca/gui/properties/clickable.hpp"
#include "ca/gui/properties/moveable.hpp"
#include "ca/gui/properties/refframe.hpp"
#include "ca/gui/properties/resizeable.hpp"
#include "ca/pa/memory/refptr.hpp"

namespace ca { namespace gui {

	class Widget;
	typedef pa::RefPtr<Widget> WidgetPtr;
	typedef pa::RefPtr<const Widget> ConstWidgetPtr;

	/// Base class with mandatory attributes for all widgets.
	/// \details A widget only contains the state. State handling in general is up to the derived
	///		elements.
	class Widget: public pa::ReferenceCountable, public Anchorable
	{
	public:
		Widget();
		virtual ~Widget();

		/// Set the button width and heigh (resets anchoring)
		void setSize(const Coord2& _size);
		Coord2 getSize() const;

		/// Set the position of the lower left corner (resets anchoring)
		void setPosition(const Coord2& _position);
		Coord2 getPosition() const;

		/// Move the component and reset anchoring if necessary. Also causes OnExtentChanged() if
		/// _deltaPosition is unequal zero.
		void move(const Coord2& _deltaPosition);

		/// Set both: position and size (resets anchoring)
		void setExtent(const Coord2& _position, const Coord2& _size);

		/// Change the size of one or multiple sides and reset anchoring if necessary.
		/// \details Calls onExtentChanged if something changed.
		void resize(float _deltaLeft, float _deltaRight, float _deltaBottom, float _deltaTop);

		/// Get the bounding box of the component
		const RefFrame& getRefFrame() const { return m_refFrame; }

		/// Draw the element now
		virtual void draw() const = 0;

		/// Process mouse input if desired.
		/// \details The Widget implementation forwards input to click-, resize- and move-
		///		components (in this order). It should be called from any inheriting class.
		/// \return True if this component absorbed the input. If any compontent returns true
		///		no other component will be checked afterwards.
		virtual bool processInput(const struct MouseState& _mouseState);

		/// Process Keyboard input if desired.
		/// \return True if this component absorbed the input. If any compontent returns true
		///		no other component will be checked afterwards.
		virtual bool processInput(const struct KeyboardState& _mouseState) { return false; }

		/// Can this element currently receive input (enabled)?
		bool isEnabled() const { return m_enabled; }
		void enable() { m_enabled = true; }
		void disable() { m_enabled = false; }
		void setEnabled(bool _state) { m_enabled = _state; }

		/// Is the current element visible/hidden?
		bool isVisible() const { return m_visible; }
		void show() { if(!m_visible) { m_visible = true; if(onVisibilityChanged) onVisibilityChanged(this); } }
		void hide() { if(m_visible) { m_visible = false; if(onVisibilityChanged) onVisibilityChanged(this); } }
		void setVisible(bool _state) { if(m_visible != _state) { m_visible = _state; if(onVisibilityChanged) onVisibilityChanged(this); } }

		/// Some functions which are triggered on visiblity events.
		/// The OnVisibilityChanged is called after the visibility state of the element changed.
		/// \param [in] _this This widget (the one which is shown/hidden).
		typedef std::function<void(Widget* _this)> OnVisibilityChanged;
		void setOnVisibilityChangedFunc(OnVisibilityChanged _callback) { onVisibilityChanged = _callback; }
		/// The OnPopup function is called after the OnVisibilityChanged in case the
		/// trigger was a popup show (not on hide).
		/// \param [in] _this This widget (the one which is shown).
		/// \param [in] _originator The element whose popup is shown.
		typedef std::function<void(Widget* _this, const Widget* _originator)> OnPopup;
		void setOnPopupFunc(OnPopup _callback) { onPopup = _callback; }
		void showAsPopup(const Widget* _originator) { show(); if(onPopup) onPopup(this, _originator); }

		/// True if the mouse is on the interaction region and the element is focused by
		/// the GUIManager. I.e. there is no other element in front of this one.
		bool isMouseOver() const;

		/// Can this element get the focus?
		bool isKeyboardFocusable() const { return m_keyboardFocusable; }

		const Widget* parent() const { return m_parent; }
		Widget* parent() { return m_parent; }

		/// Let this widget create anchors for others
		void setAnchorProvider(std::unique_ptr<IAnchorProvider> _anchorProvider);
		/// Get the anchor component (can be nullptr)
		IAnchorProvider* getAnchorProvider() const	{ return m_anchorProvider.get(); }

		/// Every component has an interactio region. If not explicitly set this is the reference frame.
		virtual const IRegion* getRegion() const;
		/// Set a handler to detect if the mouse is in the region of this object.
		/// This setter also registers the region in the click-componen if existent.
		/// \param [in] _region Valid interaction-regions are CircleRegion and MaskRegion.
		///
		///		If nothing is set/the nullptr is given, the own reference frame defines the region.
		/// \param [in] _delete The region object must be deleted.
		void setRegion(std::unique_ptr<IRegion> _region);

		/// Add or remove keyboard-focus property (this element can be focused by the toggle-focus-key).
		void setKeyboardFocusable(bool _enable) { m_keyboardFocusable = _enable; }

		/// Callback which is triggered if the element receives or looses the keyboard focus
		typedef std::function<void(Widget* _this, bool _gotKeyboardFocus)> OnKeyboardFocus;
		void setOnKeyboardFocusFunc(OnKeyboardFocus _callback) { m_onKeyboardFocus = _callback; }

		/// Realign component to its anchors. If there is no anchor-component do nothing.
		virtual void refitToAnchors();

		/// Set another widget as the info popup. Info popups are opened and closed
		/// by the GUIManager automatically.
		/// \details The component will be hidden after the call (invariant of popups).
		void setInfoPopup(WidgetPtr _popup) { m_infoPopup = _popup; _popup->hide(); }
		WidgetPtr& getInfoPopup() { return m_infoPopup; }

		/// A number which can be used by the user to attach some extra information.
		/// If you encode a pointer make sure you handle the memory somewhere else.
		uint64 metaData = 0;

		/// Enable calls to processInput of a subcomponent.
		/// \details This is meant to be used in the constructor of IMouseProcessAble components.
		void registerMouseInputComponent(IMouseProcessAble* _component);
	protected:
		bool m_enabled;					///< The element can currently receive input (not disabled).
		bool m_keyboardFocusable;		///< Can this object have the focus?
		bool m_visible;					///< Draw the element if visible and mask input otherwise.

		RefFrame m_refFrame;
		// List of optional components (can be nullptr)
		IRegion* m_region;				///< Special interaction region.
		std::unique_ptr<IRegion> m_regionDeallocator;
		std::unique_ptr<IAnchorProvider> m_anchorProvider;
		std::vector<IMouseProcessAble*> m_mouseInputSubcomponents;	///< A list of behavior components to handle mouse inputs. The order is determined by inheritance order.
		IMouseProcessAble* m_activeComponent;	///< Some components want to have exclusive mouse input once they started. If this is != nullptr it will be executed as the first one.

		WidgetPtr m_infoPopup;

		Widget* m_parent;
		friend class Group; // Parent must be set from someone
		friend class GUIManager; // To trigger events

		/// Optional method to react on resize events. This is necessary to reset provided anchors.
		virtual void onExtentChanged(bool _positionChanged, bool _sizeChanged);
		OnVisibilityChanged onVisibilityChanged;
		OnPopup onPopup;
		virtual void onKeyboardFocus(bool _gotFocus);
		OnKeyboardFocus m_onKeyboardFocus;
	};

}} // namespace ca::gui