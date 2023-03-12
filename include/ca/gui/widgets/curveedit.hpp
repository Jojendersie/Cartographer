#pragma once

#include "widget.hpp"

namespace ca { namespace gui {

	/// Simple text element
	class CurveEdit : public Widget, public IMouseProcessAble
	{
		struct Handle {
			ei::Vec2 domainPos;
			ei::Vec2 screenPos;
			ei::Vec2 tangentLeft;		// Domain space
			ei::Vec2 tangentRight;		// Domain space
			ei::Vec2 screenHdlLeft;		// Screen space
			ei::Vec2 screenHdlRight;	// Screen space
			bool tangentsLocked;
		};

	public:
		// In any mode the curve will always be a function (one y for each x).
		enum class Mode {
			LINEAR,			// Linear segments between the vertices
			CUBIC_HERMITE,	// Smooth curve where tangents define directions, but not stiffness
			BEZIER			// Full control with handles.
		};

		CurveEdit();

		/// Implement the draw method
		void draw() const override;

		/// \param [in] _color Set an individual RGB color (0-1) including alpha value
		/// 	for the background.
		void setBackgroundColor(const ei::Vec4& _color) { m_backgroundColor = _color; }
		const ei::Vec4& getBackgroundColor() const { return m_backgroundColor; }

		/// \param [in] _color Set an individual RGB color (0-1) including alpha value
		///		for the grid lines (if they are used).
		void setGridColor(const ei::Vec4& _color) { m_gridColor = _color; }
		const ei::Vec4& getGridColor() const { return m_gridColor; }

		/// \param [in] _color Set an individual RGB color (0-1) including alpha value
		///		for the curve lines.
		void setCurveColor(const ei::Vec4& _color) { m_curveColor = _color; }
		const ei::Vec4& getCurveColor() const { return m_curveColor; }

		/// Set the space between grid lines (in domain space).
		/// 0 deactivates rendering of the grid in that dimension.
		void setGridSpace(ei::Vec2 _size) { m_gridSpace = _size; }

		/// Set the value domain for the curve handle positions.
		/// The entire domain plus margins will be stretched to the rendering area.
		void setDomain(const ei::Vec2& _xRange, const ei::Vec2& _yRange);

		/// Additional space in the domain. Handle positions are restricted to the exact
		/// domain, but tangent vectors may reach into the margin.
		/// All values must be positive (additonal space) or 0
		void setDomainMargin(float _xNeg, float _xPos, float _yNeg, float _yPos);

		/// Change the rendering and input mode
		void setMode(Mode _mode);

		/// Move handles etc.
		bool processInput(
			class Widget& _thisWidget,
			const struct MouseState& _mouseState,
			bool _cursorOnWidget,
			bool& _ensureNextInput
		) override;

		/// React to movements or rotations of handles.
		/// The tangents are not used in linear mode.
		typedef std::function<void(int _idx, const ei::Vec2& _pos, const ei::Vec2& _tangentLeft, const ei::Vec2& _tangentRight)> OnHandleChanged;
		void setOnHandleChangedFunc(OnHandleChanged _callback) { m_onHandleChanged = _callback; }

		/// Functions to signal if handles are added or removed.
		typedef std::function<void(int _idx)> OnHandleDeleted;
		void setOnNewHandleFunc(OnHandleChanged _callback) { m_onNewHandle = _callback; }
		void setOnDeleteHandle(OnHandleDeleted _callback) { m_onDeleteHandle = _callback; }

		/// Add a new handle at the specified index (gets clammped).
		int addHandle(const ei::Vec2& _pos, const ei::Vec2& _tangentLeft, const ei::Vec2& _tangentRight);

		/// Deletes the handle at the index and does nothing if the index is out of range.
		void deleteHandle(int _idx);

		// Delete all handles
		void clear();

		int getNumHandles() const { return (int)m_handles.size(); }
		const Handle& getHandle(int _idx) const { return m_handles[_idx]; }

		void onExtentChanged() override;
	private:
		OnHandleChanged m_onHandleChanged;
		OnHandleChanged m_onNewHandle;
		OnHandleDeleted m_onDeleteHandle;
		ei::Vec4 m_backgroundColor;
		ei::Vec4 m_gridColor;
		ei::Vec4 m_curveColor;
		std::vector<Handle> m_handles;
		ei::Vec2 m_gridSpace;
		ei::Vec2 m_xDomain, m_yDomain;
		ei::Vec2 m_xRange, m_yRange;	// Domain + marigin
		Mode m_mode;

		// Temporary input handling
		int m_selectedHdl;
		int m_selectedSubHdl;
		ei::Vec2 m_domainToScreen;
		ei::Vec2 m_screenToDomain;
		ei::Vec2 m_screenOffset;
		ei::Vec2 m_domainOffset;
		Handle m_selectedCopy;	// A copy to detect if the handle changed at the end of input

		void recomputeSpaceConversions();
		void recomputeScreenPos(int hdl, bool pos = true, bool left = true, bool right = true);
		void limitHdl(int hdl, bool left, bool right);
	};

	typedef pa::RefPtr<CurveEdit> CurveEditPtr;

}} // namespace ca::gui
