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

		void onExtentChanged() override;

		/// Process mouse input for drag & drop like movement of the slider.
		/// \param [in] _mouseState State of the mouse buttons and position. Required to start/end
		///		movements.
		bool processInput(Widget & _thisWidget, const struct MouseState & _mouseState, bool _cursorOnWidget, bool & _ensureNextInput) override;

		/// Set if the bar should move (and render) horizontal or vertical.
		/// The default is false (vertical scrolling)
		void setHorizontalMode(const bool _horizontal);
		bool isHorizontal() const { return m_horizontal; }

		/// Set two reference points that will automatically adjust with the provided anchor reference.
		/// Those points will change with the area where the actual content is shown.
		void setAvailableArea(IAnchorProvider* _area, Coord _minPos, Coord _maxPos);

		/// When setting the content size, it is assumed that the given interval includes current
		/// translations through active scrolling offset.
		void setContentInterval(const Coord _min, const Coord _max);

		/// Get the offset how much the content extends below the available interval start.
		/// If the content is outside the number is negative, if it is moved towards the end
		/// of the available area, the number is positive.
		//void setScrollOffset(const float _amount);
		float getScrollOffset() const { return m_contentInterval.x; }

		/// Like setScrollOffset() but measures distance beginning at end of the
		/// available interval.
		//void setScrollOffsetTop(const float _amount);
		float getScrollOffsetTop() const { return m_contentInterval.y + m_availableInterval.x - m_availableInterval.y; }

		/// Returns a special anchor provider of a frame that moves around.
		/// \details This anchor frame has to be used for things that should move
		///		on scrolling.
		const IAnchorProvider* getAnchor() const	{ return &m_sliderAnchor; }

		/// Get the available size either from the presentation widget or from
		/// absolute setting (whatever is used).
		float getAvailableSize() const;
		/// Get the content interval in absolute coordinates.
		/// This reflects the current position including scrolling translation.
		Coord2 getContentInterval() const;

		/// Set a function to react to changes of the scroll interval.
		/// \param [in] _this This widget (the scrollbar).
		/// \param [in] _amount Positive or negative delta of the change. Guaranteed to be not zero.
		typedef std::function<void(Widget* _this, const float _amount)> OnChange;
		void setOnChangeFunc(OnChange _callback) { m_onChange = _callback; }

	private:
		class SliderAnchor : public IAnchorable, public IAnchorProvider
		{
		public:
			ScrollBar* m_parent;	///< Link back to scrollbar to react to presentation widget changes.
			Anchor m_anchor;		///< The anchor to move m_position with some reference.

			SliderAnchor(ScrollBar* _parent);
			void attach(const IAnchorProvider* _target); ///< Absolute anchoring against a new reference
			void setAnchor(float _offset);
			void moveAnchor(float _offset);
			void onExtentChanged() override;
			Coord getPosition(int _dimension, float _relativePos) const override;
			float getRelativePosition(int _dimension, Coord _position) const override;
			void refitToAnchors() override;
			void resetAnchors() override {}
		};

		mutable SliderAnchor m_sliderAnchor;		///< Special area that moves around on slide
		Anchor m_availableStart;	///< An anchor that is linked to some component on which the content is shown. Marks the lower end of the interval.
		Anchor m_availableEnd;		///< An anchor that is linked to some component on which the content is shown. Marks the upper end of the interval.
		OnChange m_onChange;	///< Gets called if m_intervalStart changes
		bool m_horizontal;		///< Horizontal or vertical mode?
		float m_movingPos;		///< Relative position within the content section bar that is moved. (Or -1 if not moving)

		// The total range, represented by the scrollbar, is the union of the following interval and [0, m_availableSize].
		Coord2 m_availableInterval;		///< Last known available interval infered from the anchors
		Coord2 m_contentInterval;		///< Bounding interval of the content relative to m_availableInterval.x
	};

	typedef pa::RefPtr<ScrollBar> ScrollBarPtr;
	typedef pa::RefPtr<const ScrollBar> ConstScrollBarPtr;

}} // namespace ca::gui
