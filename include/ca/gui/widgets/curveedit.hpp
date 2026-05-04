#pragma once

#include "widget.hpp"

namespace ca { namespace gui {

	/// Simple text element
	class CurveEdit : public Widget, public IMouseProcessAble
	{
		struct Handle {
			ei::Vec2 screenPos;
			ei::Vec2 screenHdlLeft;		// Screen space
			ei::Vec2 screenHdlRight;	// Screen space
			bool tangentsLocked;
		};

	public:
		enum Flags {
			// Render tangents, but do not interact with them.
			SHOW_TANGENTS = 1,
			// If set, tangents will be rendered and can be edited. (Implies SHOW_TANGENT)
			TANGENT_EDITING = 3,
			// If set, double clicking a node will toggle its type (e.g. between smooth and cusp).
			TOGGLE_NODE_TYPE = 4,
			// New nodes can be added by double clicking in empty space.
			ADD_NODES = 8,
			// Nodes can be removed by double right clicking on them.
			REMOVE_NODES = 16,
		};

		CurveEdit();

		/// Set what the editor will render and which edit options are available.
		void setMode(Flags _modeFlags) { m_flags = _modeFlags; }

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

		/// Move handles etc.
		bool processInput(
			class Widget& _thisWidget,
			const struct MouseState& _mouseState,
			bool _cursorOnWidget,
			bool& _ensureNextInput
		) override;


		/// Interface to be implemented by the user to map the real model to the UI.
		/// All coordinates read and written into the curve handler are always in curve domain
		/// (never screen domain).
		class ICurveDataHandler : public ReferenceCountable
		{
		public:
			/// Fetch the position of a vertex.
			virtual ei::Vec2 getPosition(int _idx) const = 0;

			/// [Optional] Fetch a tangent vector on one side of a vertex.
			/// This method is only called if TANGENT_EDITING is enabled.
			virtual ei::Vec2 getTangent(int _idx, bool _left) const { return {_left?-1.0f:1.0f, 0.0f}; }

			/// Function to evaluate the represented curve at an specific x coordinate.
			///  _idx: The index of the vertex with the largest x coordinate smaller or equal _x.
			///  _x: Domain x coordinate. May be equal or larger than the given vertex's position.
			virtual float getValue(int _idx, float _x) const = 0;

			/// Send changes of the position to the model.
			/// After the call, this editor will refetch the positions and tangents, such that the model
			/// may restore arbitrary invariants.
			virtual void onPositionChanged(int _idx, const ei::Vec2& _newPos) = 0;

			/// [Optional] Send changes of a tangent to the model.
			virtual void onTangentChanged(int _idx, const ei::Vec2& _newPos, bool _left) {};

			/// Functions to signal if handles are added or removed.
			/// onNewHandle may return false if no handle was added.
			virtual bool onNewHandle(int _idx, const ei::Vec2& _newPos) = 0;
			virtual void onDeleteHandle(int _idx) = 0;
		};

		/// Set an implementation that maps the curve model and the ui.
		void setCurveDataHandler(pa::RefPtr<ICurveDataHandler> _curveHandler) { m_curve = _curveHandler; }

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
		Flags m_flags;
		pa::RefPtr<ICurveDataHandler> m_curve;
		ei::Vec4 m_backgroundColor;
		ei::Vec4 m_gridColor;
		ei::Vec4 m_curveColor;
		std::vector<Handle> m_handles;
		ei::Vec2 m_gridSpace;			// Domain sided distance between two lines
		ei::IVec2 m_labelPrecision;		// Number of decimal places for labels. Computed from grid space to distinguish the values.
		ei::Vec2 m_xDomain, m_yDomain;
		ei::Vec2 m_xRange, m_yRange;	// Domain + margin

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
		void updateHandles();
	};

	typedef pa::RefPtr<CurveEdit> CurveEditPtr;

}} // namespace ca::gui
