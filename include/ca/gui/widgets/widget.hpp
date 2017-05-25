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

	/// Base class with mandatory attributes for all widgets.
	/// \details A widget only contains the state. State handling in general is up to the derived
	///		elements.
	class Widget: public pa::ReferenceCountable
	{
	public:
		Widget(bool _anchorable = false, bool _clickable = false, bool _moveable = false, bool _resizeable = false);
		virtual ~Widget() = default;

		/// Set the button width and heigh (resets anchoring)
		void setSize(const Coord2& _size);
		Coord2 getSize() const;

		/// Set the position of the lower left corner (resets anchoring)
		void setPosition(const Coord2& _position);
		Coord2 getPosition() const;

		/// Set both: position and size (resets anchoring)
		void setExtent(const Coord2& _position, const Coord2& _size);

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

		/// Can this element currently receive input (enabled)?
		bool isEnabled() const { return m_enabled; }
		void enable() { m_enabled = true; }
		void disable() { m_enabled = false; }
		void setEnabled(bool _state) { m_enabled = _state; }

		/// Is the current element visible/hidden?
		bool isVisible() const { return m_visible; }
		void show() { m_visible = true; }
		void hide() { m_visible = false; }
		void setVisible(bool _state) { m_visible = _state; }

		/// True if the mouse is on the interaction region and the element is focused by
		/// the GUIManager. I.e. there is no other element in front of this one.
		bool isMouseOver() const;

		/// Can this element get the focus?
		bool isKeyboardFocusable() const { return m_keyboardFocusable; }

		const Widget* parent() const { return m_parent; }
		Widget* parent() { return m_parent; }

		/// Attach one of the four reference sides to an anchor point
		/// \details If the element was created without anchoring this fails with an error message.
		void setAnchoring(SIDE::Val _side, AnchorPtr _anchor);
		void setHorizontalAnchorMode(Anchorable::Mode _mode);
		void setVerticalAnchorMode(Anchorable::Mode _mode);
		void setAnchorModes(Anchorable::Mode _mode);
		void setAnchorModes(Anchorable::Mode _horizontalMode, Anchorable::Mode _verticalMode);
		/// Automatically attach all four anchor points to the closest anchors in the provider.
		/// \details Fails silently if the component is not anchorable
		void autoAnchor(const class IAnchorProvider* _anchorProvider);
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

		/// Add or remove the clickable property (you can add CLICK and DBLCLICK callbacks)
		/// The old click component is kept if there is already one.
		void setClickable(bool _enable);
		/// Add or remove the anchor property (this element can be anchored).
		/// The old anchor component is kept if there is already one.
		void setAnchorable(bool _enable);
		/// Add or remove the move property (this element is moved, if mouse is pressed and then moved).
		/// The old move component is kept if there is already one. The resize and move
		/// components are updated if necessary.
		void setMoveable(bool _enable);
		/// Add or remove the resize property (this element can be resized by mouse).
		/// The old resize component is kept if there is already one.
		void setResizeable(bool _enable);
		/// Add or remove keyboard-focus property (this element can be focused by the toggle-focus-key).
		void setKeyboardFocusable(bool _enable) { m_keyboardFocusable = _enable; }

		Moveable* getMoveComponent() { return m_moveComponent.get(); }
		const Moveable* getMoveComponent() const { return m_moveComponent.get(); }

		/// Realign component to its anchors. If there is no anchor-component do nothing.
		virtual void refitToAnchors();

		/// A number which can be used by the user to attach some extra information.
		/// If you encode a pointer make sure you handle the memory somewhere else.
		uint64 metaData = 0;
	protected:
		bool m_enabled;					///< The element can currently receive input (not disabled).
		bool m_keyboardFocusable;		///< Can this object have the focus?
		bool m_visible;					///< Draw the element if visible and mask input otherwise.

		RefFrame m_refFrame;
		// List of optional components (can be nullptr)
		IRegion* m_region;				///< Special interaction region.
		std::unique_ptr<IRegion> m_regionDeallocator;
		std::unique_ptr<Anchorable> m_anchorComponent;
		std::unique_ptr<Clickable> m_clickComponent;
		std::unique_ptr<Moveable> m_moveComponent;
		std::unique_ptr<Resizeable> m_resizeComponent;
		std::unique_ptr<IAnchorProvider> m_anchorProvider;

		Widget* m_parent;
		friend class Group; // Parent must be set from someone

		/// Optional method to react on resize events. This is necessary to reset provided anchors.
		virtual void onExtentChanged(bool _positionChanged, bool _sizeChanged);
	};

	typedef pa::RefPtr<Widget> WidgetPtr;

}} // namespace ca::gui