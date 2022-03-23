#pragma once

#include "widget.hpp"

namespace ca { namespace gui {

	/// Horizontal and vertical mode scrollbar that provides scrolling within a frame
	/// by providing an anchor. The default mode is vertical.
	class ScrollBar : public Widget, public IMouseProcessAble
	{
	public:
		ScrollBar();
		~ScrollBar();

		/// Implement the draw method
		void draw() const override;

		/// Process mouse input for drag & drop like movement of the slider.
		/// \param [in] _mouseState State of the mouse buttons and position. Required to start/end
		///		movements.
		bool processInput(Widget & _thisWidget, const struct MouseState & _mouseState, bool _cursorOnWidget, bool & _ensureNextInput) override;

		/// Set if the bar should move (and render) horizontal or vertical.
		/// The default is false (vertical scrolling)
		void setHorizontalMode(const bool _horizontal);
		bool isHorizontal() const { return m_horizontal; }

		/// Set arbitrary size limits. The available size corresponds to the visible
		/// area. Alternatively to setting constants, it is possible to assign reference
		/// widgets for both sizes (eg. a frame and a group/single content element)
		void setAvailableSize(const float _availableSize);
		/// Set the size of the scrollable content. If a content widget was attached
		/// it will be removed.
		/// \param [in] _side Info on which side of the interval, the size change
		///		happened. In horizontal mode only LEFT/RIGHT are valid and in vertical
		///		mode BOTTOM/TOP. This is used to recompute the proper scroll section
		///		position.
		void setContentSize(const float _contentSize, SIDE::Val _side);

		/// Set a widget that is used to infer the available area.
		/// _margin: The margin reduces the absolute size of the available
		///		area. Only positive values are allowed. Clipping or placement
		///		are not handled by the scrollbar. The margin only gives space
		///		and extends the scrolling distance.
		void setViewArea(WidgetPtr _presentationWidget, const float _margin);
		void setContent(WidgetPtr _contentWidget);

		/// Set or get the offset, which is number in [0, totalSize-availableSize]
		/// that can be used to move the content of totalSize extent such that
		/// either the beginning or the end (or both) still fit into availableSize.
		/// Note that the recommended usage is to use the provided anchors.
		void setScrollOffset(const float _amount);
		float getScrollOffset() const { return m_intervalStart; }

		/// Returns the anchor from the contained single point anchor provider.
		/// This anchor has to be used for things that should move on scrolling.
		AnchorPtr getAnchor() const;

		/// Get the available size either from the presentation widget or from
		/// absolute setting (whatever is used).
		float getAvailableSize() const;
		float getTotalSize() const;

	private:
		WidgetPtr m_presentationWidget;
		WidgetPtr m_contentWidget;
		bool m_horizontal;		//< Horizontal or vertical mode?
		float m_totalSize;		//< Size of the area that is scrolled
		float m_availableSize;	//< Size of the view that contains the scrolled content
		float m_intervalStart;	//< Position of scrolling.
		float m_rangeOffset;	//< Can move intervalStart into the negative range if things are added left/bottom
		float m_margin;			//< Positive distances to all sides.
		float m_movingPos;		//< Relative position within the content section bar that is moved. (Or -1 if not moving)

		void checkInterval(const bool _forceAnchorReset = false);
		void refitToAnchors() override;

		/// An anchor provider that is a single point in a single dimension
		class SingleAnchorProvider : public IAnchorProvider
		{
		public:
			SingleAnchorProvider(ScrollBar* _parent);
			~SingleAnchorProvider();

			void recomputeAnchors(const class RefFrame& _selfFrame) override;
			AnchorPtr findClosestAnchor(Coord _position, SearchDirection _direction) const override;

		private:
			std::shared_ptr<AnchorPoint> m_anchor;
			ScrollBar* m_parent;
		};

		// Hide the method that could damage the setup (although sombody could still do that
		// using the base class...
		using Widget::setAnchorProvider;
	};

	typedef pa::RefPtr<ScrollBar> ScrollBarPtr;
	typedef pa::RefPtr<const ScrollBar> ConstScrollBarPtr;

}} // namespace ca::gui
