#pragma once

#include "widget.hpp"

namespace ca { namespace gui {

	/// Simple text element
	class CurveEdit : public Widget, public IMouseProcessAble
	{
		struct Handle {
			ei::Vec2 screenPos;
			ei::Vec2 screenTangentLeft;
			ei::Vec2 screenTangentRight;
			ei::Vec2 screenHdlLeft;		// Screen space
			ei::Vec2 screenHdlRight;	// Screen space
			bool tangentsLocked;
		};

	public:
		// In any mode the curve will always be a function (one y for each x).
		enum class Mode {
			LINEAR,			// Linear segments between the vertices
			SMOOTH,			// Cubic spline with automatic tangents (don't show the tangents/handles)
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
		void setGridSpace(ei::Vec2 _size) { m_gridSpace = _size; m_labelPrecision = ei::IVec2{ei::max(0, -ei::floor(log10f(m_gridSpace.x))), ei::max(0, -ei::floor(log10f(m_gridSpace.y)))}; }

		/// Set the value domain for the curve handle positions.
		/// The entire domain plus margins will be stretched to the rendering area.
		void setDomain(const ei::Vec2& _xRange, const ei::Vec2& _yRange);

		/// Additional space in the domain. Handle positions are restricted to the exact
		/// domain, but tangent vectors may reach into the margin.
		/// All values must be positive (additonal space) or 0
		void setDomainMargin(float _xNeg, float _xPos, float _yNeg, float _yPos);

		/// Change the rendering and input mode
		void setMode(Mode _mode, bool _periodic);

		/// Set the rendering tangent length in pixels (only applies to Mode::CUBIC_HERMITE)
		void setTangentLength(float _length) { m_tangentLength = _length; }

		/// Move handles etc.
		bool processInput(
			class Widget& _thisWidget,
			const struct MouseState& _mouseState,
			bool _cursorOnWidget,
			bool& _ensureNextInput
		) override;

		/// Functions to fetch tangent vectors/slopes and positions from the model.
		/// If the GetTangent method is used depends on the selected model:
		///		LINEAR will not call it
		///		SMOOTH / CUBIC_HERMITE will call GetTangent once (always the right tangent, i.e. _left=false)
		/// 	BEZIER will call GetTangent twice, because they may be independent
		typedef std::function<ei::Vec2(int _idx)> GetPosition;
		typedef std::function<ei::Vec2(int _idx, bool _left)> GetTangent;
		void setGetPositionFunc(GetPosition _callback) { m_getPosition = _callback; }
		void setGetTangentFunc(GetTangent _callback) { m_getTangent = _callback; }

		/// Send changes of the position to the model.
		/// The callback must return which handles are hanged by this setting. An interval where x > y
		/// is allowed and wraps around on the right side. This allows the model to restore its invariants
		/// for tangents and positions.
		/// After the call, this editor will refetch the positions and tangents of the changed nodes.
		typedef std::function<ei::IVec2(int _idx, const ei::Vec2& _newPos)> OnHandleChanged;
		void setOnPositionChangedFunc(OnHandleChanged _callback) { m_onPositionChanged = _callback; }

		/// Send changes of a tangent to the model.
		/// These are not used for the linear and smooth models.
		/// Both functions should be set even if the model only allows coupled tangents.
		typedef std::function<ei::IVec2(int _idx, const ei::Vec2& _vec, bool _left)> OnTangentChanged;
		void setOnTangentChanged(OnTangentChanged _callback) { m_onTangentChanged = _callback; }

		/// Functions to signal if handles are added or removed.
		/// Must return the interval of changed nodes (without the one being removed).
		typedef std::function<ei::IVec2(int _idx)> OnHandleDeleted;
		void setOnNewHandleFunc(OnHandleChanged _callback) { m_onNewHandle = _callback; }
		void setOnDeleteHandle(OnHandleDeleted _callback) { m_onDeleteHandle = _callback; }

		/// Add one or multiple handles. The given index will be clamped to the valid
		/// possible range. The function will call getPosition and getTangent for the new points
		/// and for the adjacent points if there are any.
		/// Does not trigger on handle changed functions
		void addHandles(int _idx, int _num);

		/// Deletes the handle at the index and does nothing if the index is out of range.
		void deleteHandle(int _idx);

		// Delete all handles
		void clear();

		int getNumHandles() const { return (int)m_handles.size(); }
		const Handle& getHandle(int _idx) const { return m_handles[_idx]; }

		void onExtentChanged() override;
	private:
		OnHandleChanged m_onNewHandle;
		OnHandleDeleted m_onDeleteHandle;
		GetTangent m_getTangent;
		GetPosition m_getPosition;
		OnHandleChanged m_onPositionChanged;
		OnTangentChanged m_onTangentChanged;
		ei::Vec4 m_backgroundColor;
		ei::Vec4 m_gridColor;
		ei::Vec4 m_curveColor;
		std::vector<Handle> m_handles;
		ei::Vec2 m_gridSpace;			// Domain sided distance between two lines
		ei::IVec2 m_labelPrecision;		// Number of decimal places for labels. Computed from grid space to distinguish the values.
		ei::Vec2 m_xDomain, m_yDomain;
		ei::Vec2 m_xRange, m_yRange;	// Domain + margin
		Mode m_mode;
		bool m_periodic;				// Match endpoints
		float m_tangentLength;			// Length of a tangent vector in hermite mode

		// Temporary input handling
		int m_selectedHdl;
		int m_selectedSubHdl;
		ei::Vec2 m_domainToScreen;
		ei::Vec2 m_screenToDomain;
		ei::Vec2 m_screenOffset;
		ei::Vec2 m_domainOffset;
		Handle m_selectedCopy;	// A copy to detect if the handle changed at the end of input

		void recomputeSpaceConversions();
		// Fetch tangents and positions for the respective interval of handles (right boundary exclusive)
		void updateHandles(const ei::IVec2& interval);
	};

	typedef pa::RefPtr<CurveEdit> CurveEditPtr;

}} // namespace ca::gui
