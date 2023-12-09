#include "ca/gui/widgets/curveedit.hpp"
#include "ca/gui/guimanager.hpp"
#include "ca/gui/rendering/theme.hpp"
#include "ca/gui/backend/renderbackend.hpp"

using namespace ei;

namespace ca { namespace gui {

	CurveEdit::CurveEdit() :
		m_onHandleChanged { nullptr },
		m_onNewHandle { nullptr },
		m_onDeleteHandle { nullptr },
		m_backgroundColor { 0.0f },
		m_gridColor { 0.1f },
		m_curveColor { 0.1f, 0.7f, 0.1f, 1.0f },
		m_gridSpace { 0.1f },
		m_xDomain {0.0f, 1.0f },
		m_yDomain {0.0f, 1.0f },
		m_xRange {-0.1f, 1.1f },
		m_yRange {-0.1f, 1.1f },
		m_mode { Mode::BEZIER },
		m_selectedHdl {-1},
		m_selectedSubHdl {-1}
	{
		Widget::registerMouseInputComponent(this);
		recomputeSpaceConversions();
	}


	constexpr Vec4 hemite_spline_coeffs(const float t)
	{
		return Vec4{ (1+2*t)*(1-t)*(1-t),
			t*(1-t)*(1-t),
			t*t*(3-2*t),
			-t*t*(1-t) };
	}

	void CurveEdit::draw() const
	{
		if(m_backgroundColor.a > 0.0f)
		{
			GUIManager::theme().drawBackgroundArea(rectangle(), m_backgroundColor.a, Vec3(m_backgroundColor));
			// Make the domain background different
			Rect2D domainFrame {
				position() + Vec2{round((m_xDomain.x - m_xRange.x) * m_domainToScreen.x), round((m_yDomain.x - m_yRange.x) * m_domainToScreen.y)},
				position() + Vec2{round((m_xDomain.y - m_xRange.x) * m_domainToScreen.x), round((m_yDomain.y - m_yRange.x) * m_domainToScreen.y)}+1.0f
			};
			GUIManager::theme().drawBackgroundArea(domainFrame, m_backgroundColor.a, Vec3(m_backgroundColor) * 0.5f);
		}

		if(m_gridColor.a > 0.0f && m_gridSpace.x > 0.0f)
		{
			const int firstTick = ei::ceil(m_xRange.x / m_gridSpace.x);
			const int lastTick = ei::floor(m_xRange.y / m_gridSpace.x);
			const float firstTickScreen = left() + (firstTick * m_gridSpace.x - m_xRange.x) * m_domainToScreen.x;
			const float tickSpaceScreen = m_gridSpace.x * m_domainToScreen.x;
			for(int x = 0; x <= (lastTick - firstTick); ++x)
			{
				const float screenX = round(firstTickScreen + x * tickSpaceScreen)+0.5f;
				Vec2 pos[2] = {
					Vec2{screenX, bottom()},
					Vec2{screenX, top()}
				};
				GUIManager::theme().drawLine(pos, 2, m_gridColor, m_gridColor);
			}
		}

		if(m_gridColor.a > 0.0f && m_gridSpace.y > 0.0f)
		{
			const int firstTick = ei::ceil(m_yRange.x / m_gridSpace.y);
			const int lastTick = ei::floor(m_yRange.y / m_gridSpace.y);
			const float firstTickScreen = bottom() + (firstTick * m_gridSpace.y - m_yRange.x) * m_domainToScreen.y;
			const float tickSpaceScreen = m_gridSpace.y * m_domainToScreen.y;
			for(int y = 0; y <= (lastTick - firstTick); ++y)
			{
				const float screenY = round(firstTickScreen + y * tickSpaceScreen)+0.5f;
				Vec2 pos[2] = {
					Vec2{left(), screenY},
					Vec2{right(), screenY}
				};
				GUIManager::theme().drawLine(pos, 2, m_gridColor, m_gridColor);
			}
		}

		// Draw the lines of the plot
		if(m_mode == Mode::LINEAR)
		{
			Vec2 buf[2];
			for(size_t i = 0; i < m_handles.size()-1; ++i)
			{
				buf[0] = Vec2 { round(m_handles[i].screenPos) };
				buf[1] = Vec2 { round(m_handles[i+1].screenPos) };
				GUIManager::theme().drawLine(buf, 2, m_curveColor, m_curveColor);
			}
		}
		else if(m_mode == Mode::CUBIC_HERMITE)
		{
			constexpr int RES = 12;
			Vec2 buf[RES];
			static constexpr Vec4 HERMITE[RES-2] = {
				hemite_spline_coeffs(1.0f / (RES-1)),
				hemite_spline_coeffs(2.0f / (RES-1)),
				hemite_spline_coeffs(3.0f / (RES-1)),
				hemite_spline_coeffs(4.0f / (RES-1)),
				hemite_spline_coeffs(5.0f / (RES-1)),
				hemite_spline_coeffs(6.0f / (RES-1)),
				hemite_spline_coeffs(7.0f / (RES-1)),
				hemite_spline_coeffs(8.0f / (RES-1)),
				hemite_spline_coeffs(9.0f / (RES-1)),
				hemite_spline_coeffs(10.0f / (RES-1))
			};
			for(size_t i = 0; i < m_handles.size(); ++i)
			{
				buf[0] = Vec2 { round(m_handles[i].screenHdlLeft) }; // TODO: move rounding into handle computation?
				buf[1] = Vec2 { round(m_handles[i].screenPos) };
				buf[2] = Vec2 { round(m_handles[i].screenHdlRight) };
				GUIManager::theme().drawLine(buf + (i == 0 ? 1 : 0), 3 - (i == 0 || i == m_handles.size()-1 ? 1 : 0), m_curveColor, m_curveColor);
				if(i > 0)
				{
					const float lSlopeR = 0.5f * m_handles[i-1].tangentRight.y / m_handles[i-1].tangentRight.x * m_domainToScreen.y;
					const float rSlopeL = 0.5f * m_handles[i].tangentLeft.y / m_handles[i].tangentLeft.x * m_domainToScreen.y;
					buf[0] = Vec2 { round(m_handles[i-1].screenPos) };
					buf[RES-1] = buf[1];
					for(int j = 0; j < RES-2; ++j)
					{
						buf[j+1] = Vec2 { lerp(m_handles[i-1].screenPos.x, m_handles[i].screenPos.x, (j+1) / float(RES-1)),
									HERMITE[j].x * m_handles[i-1].screenPos.y + HERMITE[j].y * lSlopeR + HERMITE[j].z * m_handles[i].screenPos.y + HERMITE[j].w * rSlopeL};
					}
					GUIManager::theme().drawLine(buf, RES, m_curveColor, m_curveColor);
				}
			}
		}
		else if(m_mode == Mode::BEZIER)
		{
			constexpr int RES = 12;
			Vec2 buf[RES];
			for(size_t i = 0; i < m_handles.size(); ++i)
			{
				buf[0] = m_handles[i].screenHdlLeft;
				buf[1] = m_handles[i].screenPos;
				buf[2] = m_handles[i].screenHdlRight;
				GUIManager::theme().drawLine(buf + (i == 0 ? 1 : 0), 3 - (i == 0 || i == m_handles.size()-1 ? 1 : 0), m_curveColor, m_curveColor);
				if(i > 0)
				{
					buf[0] = m_handles[i-1].screenPos;
					buf[RES-1] = buf[1];
					for(int j = 0; j < RES-2; ++j)
					{
						const float t = float(j+1)/(RES-2);
						const float it = 1.0f - t;
						buf[j+1] = m_handles[i-1].screenPos * (it * it * it) + m_handles[i-1].screenHdlRight * (it * it * t * 3.0f)
								 + m_handles[i].screenHdlLeft * (it * t * t * 3.0f) + m_handles[i].screenPos * (t * t * t);
					}
					GUIManager::theme().drawLine(buf, RES, m_curveColor, m_curveColor);
				}
			}
		}

		// Need a new layer for the handles, otherwise the lines overlap.
		GUIManager::pushClipRegion(rectangle());
		for(size_t i = 0; i < m_handles.size(); ++i)
		{
			if(m_mode == Mode::LINEAR || m_handles[i].tangentsLocked)
				GUIManager::theme().drawNodeHandle(m_handles[i].screenPos, 3.5f, Vec3{m_curveColor});
			else
			{
				GUIManager::theme().drawArrowButton(Rect2D{m_handles[i].screenPos-3.5f, m_handles[i].screenPos+3.5f}, SIDE::TOP, false); // TODO: color
			}
			if(m_mode != Mode::LINEAR)
			{
				if(i > 0)
					GUIManager::theme().drawNodeHandle(m_handles[i].screenHdlLeft, 2.5f, Vec3{m_curveColor});
				if(i < m_handles.size()-1)
					GUIManager::theme().drawNodeHandle(m_handles[i].screenHdlRight, 2.5f, Vec3{m_curveColor});
			}
		}
		GUIManager::popClipRegion();
	}


	void CurveEdit::setDomain(const ei::Vec2& _xRange, const ei::Vec2& _yRange)
	{
		m_xDomain = _xRange;
		m_yDomain = _yRange;
		for(size_t i = 0; i < m_handles.size(); ++i)
		{
			m_handles[i].domainPos.x = ei::min(ei::max(m_handles[i].domainPos.x, m_xDomain.x), m_xDomain.y);
			m_handles[i].domainPos.y = ei::min(ei::max(m_handles[i].domainPos.y, m_yDomain.x), m_yDomain.y);
			recomputeScreenPos((int)i);
		}
	}


	void CurveEdit::setDomainMargin(float _xNeg, float _xPos, float _yNeg, float _yPos)
	{
		m_xRange = m_xDomain + Vec2{-max(0.0f,_xNeg), max(0.0f,_xPos)};
		m_yRange = m_yDomain + Vec2{-max(0.0f,_yNeg), max(0.0f,_yPos)};
		recomputeSpaceConversions();
	}


	void CurveEdit::setMode(Mode _mode)
	{
		if(m_mode == _mode) return;
		if(_mode==Mode::BEZIER) // Make sure the handles are all correct
		{
			for(size_t i = 0; i < m_handles.size(); ++i)
				limitHdl((int)i, true, true);
		}
		m_mode = _mode;
	}


	bool CurveEdit::processInput(
		class Widget& _thisWidget,
		const struct MouseState& _mouseState,
		bool _cursorOnWidget,
		bool& _ensureNextInput
	) {
		const Vec2 domainPos = _mouseState.position * m_screenToDomain + m_domainOffset;
		// If something is selected, handling it has priority.
		if(m_selectedHdl != -1 || m_selectedSubHdl != -1)
		{
			// Drop the selection if mouse button is released.
			if(_mouseState.btnUp(0) || _mouseState.btnReleased(0))
			{
				const int idx = m_selectedHdl != -1 ? m_selectedHdl : m_selectedSubHdl/2;
				limitHdl(idx, true, true);

				// Notify changes if there where any
				bool changed = m_selectedCopy.domainPos != m_handles[idx].domainPos;
				if(m_mode != Mode::LINEAR)
				{
					changed |= m_selectedCopy.tangentLeft != m_handles[idx].tangentLeft;
					changed |= m_selectedCopy.tangentRight != m_handles[idx].tangentRight;
				}
				if(changed && m_onHandleChanged)
					m_onHandleChanged(idx, m_handles[idx].domainPos, m_handles[idx].tangentLeft, m_handles[idx].tangentRight);

				// If a main handle was selected, the neighbors tangents might violate the bounds
				if(m_selectedHdl != -1)
				{
					if(m_selectedHdl > 0)
						limitHdl(m_selectedHdl-1, false, true);
					if(m_selectedHdl+1 < (int)m_handles.size())
						limitHdl(m_selectedHdl+1, true, false);
				}
				m_selectedSubHdl = m_selectedHdl = -1;
			}
			// Hdl and SubHdl are exclusive, so if one is defined, the other is not.
			else if(m_selectedHdl != -1)
			{
				m_handles[m_selectedHdl].domainPos = domainPos;
				// Limit movements to neighbors
				if(m_selectedHdl > 0) m_handles[m_selectedHdl].domainPos.x = max(m_handles[m_selectedHdl].domainPos.x, m_handles[m_selectedHdl-1].domainPos.x);
				else m_handles[m_selectedHdl].domainPos.x = m_xDomain.x;
				if(m_selectedHdl+1 < (int)m_handles.size()) m_handles[m_selectedHdl].domainPos.x = min(m_handles[m_selectedHdl].domainPos.x, m_handles[m_selectedHdl+1].domainPos.x);
				else m_handles[m_selectedHdl].domainPos.x = m_xDomain.y;
				m_handles[m_selectedHdl].domainPos.y = clamp(m_handles[m_selectedHdl].domainPos.y, m_yDomain.x, m_yDomain.y);
				recomputeScreenPos(m_selectedHdl);
			}
			else // A handle is selected
			{
				const int idx = m_selectedSubHdl / 2;
				const bool selectedRight = m_selectedSubHdl & 1;
				if(m_mode == Mode::CUBIC_HERMITE)
				{
					const Vec2 tangent = normalize(domainPos - m_handles[idx].domainPos) * 0.1f; // TODO: parameter for rendering tangent length
					if(selectedRight)
						m_handles[idx].tangentRight = Vec2{ei::abs(tangent.x), tangent.y};
					else
						m_handles[idx].tangentLeft = Vec2{-ei::abs(tangent.x), tangent.y};
				} else {
					if(selectedRight)
					{
						const Vec2 domainHdl = Vec2{clamp(domainPos.x, m_handles[idx].domainPos.x, m_handles[idx+1].domainPos.x), clamp(domainPos.y, m_yDomain.x, m_yDomain.y)};
						m_handles[idx].tangentRight = domainHdl - m_handles[idx].domainPos;
					}
					else
					{
						const Vec2 domainHdl = Vec2{clamp(domainPos.x, m_handles[idx-1].domainPos.x, m_handles[idx].domainPos.x), clamp(domainPos.y, m_yDomain.x, m_yDomain.y)};
						m_handles[idx].tangentLeft = domainHdl - m_handles[idx].domainPos;
					}
				}
				if(m_handles[idx].tangentsLocked)
				{
					if(selectedRight) // Right was the original -> mirror to left
						m_handles[idx].tangentLeft = -m_handles[idx].tangentRight * sqrtf(lensq(m_handles[idx].tangentLeft) / (1e-10f +lensq(m_handles[idx].tangentRight)));
					else
						m_handles[idx].tangentRight = -m_handles[idx].tangentLeft * sqrtf(lensq(m_handles[idx].tangentRight) / (1e-10f + lensq(m_handles[idx].tangentLeft)));
				}
				recomputeScreenPos(idx, false, m_handles[idx].tangentsLocked || !selectedRight, m_handles[idx].tangentsLocked || selectedRight);
			}
		}

		// Check if the mouse is over one of the handles
		int clickedHdl = -1;
		int clickedSubHdl = -1;
		for(int i = 0; i < (int)m_handles.size(); ++i)
		{
			if(lensq(m_handles[i].screenPos - _mouseState.position) < 15.0f)
				clickedHdl = i;
			if(m_mode != Mode::LINEAR)
			{
				if(i > 0 && lensq(m_handles[i].screenHdlLeft - _mouseState.position) < 15.0f)
					clickedSubHdl = i*2;
				if(i < (int)m_handles.size() && lensq(m_handles[i].screenHdlRight - _mouseState.position) < 15.0f)
					clickedSubHdl = i*2+1;
			}
		}
		if(clickedSubHdl != -1) clickedHdl = -1;
		if(clickedHdl != -1 || clickedSubHdl != -1) GUIManager::setCursorType(CursorType::CROSSHAIR);
		else GUIManager::setCursorType(CursorType::ARROW);
		// Change the type of a handle on double click
		if(_mouseState.btnDblClicked(0) && clickedHdl != -1)
		{
			m_handles[clickedHdl].tangentsLocked = !m_handles[clickedHdl].tangentsLocked;
			if(m_handles[clickedHdl].tangentsLocked)
			{
				const Vec2 newDir = m_handles[clickedHdl].tangentRight - m_handles[clickedHdl].tangentLeft;
				const float newDirLenSq = 1e-10f + lensq(newDir);
				m_handles[clickedHdl].tangentLeft = -newDir * sqrtf(lensq(m_handles[clickedHdl].tangentLeft) / newDirLenSq);
				m_handles[clickedHdl].tangentRight = newDir * sqrtf(lensq(m_handles[clickedHdl].tangentRight) / newDirLenSq);
				recomputeScreenPos(clickedHdl, false, true, true);
			}
		}
		// Add a new handle on double click
		else if(_mouseState.btnDblClicked(0))
		{
			if(domainPos.x >= m_xDomain.x && domainPos.x <= m_xDomain.y
				&& domainPos.y >= m_yDomain.x && domainPos.y <= m_yDomain.y)
			{
				const Vec2 tangentLeft {-0.1f, 0.0f}; // TODO: infer from neighbors?
				const Vec2 tangentRight {0.1f, 0.0f}; // TODO: infer from neighbors?
				addHandle(domainPos, tangentLeft, tangentRight);
			}
		}
		else if(_mouseState.btnDblClicked(1) && clickedHdl != -1)
		{
			deleteHandle(clickedHdl);
		}

		// On button down on a handle, select it for movements.
		if(clickedHdl != -1 && _mouseState.btnDown(0))
		{
			m_selectedCopy = m_handles[clickedHdl];
			m_selectedHdl = clickedHdl;
		}
		if(clickedSubHdl != -1 && _mouseState.btnDown(0))
		{
			m_selectedCopy = m_handles[clickedSubHdl/2];
			m_selectedSubHdl = clickedSubHdl;
		}
		if(m_selectedHdl != -1 || m_selectedSubHdl != -1)
			_ensureNextInput = true;
		return true;
	}


	int CurveEdit::addHandle(const ei::Vec2& _pos, const ei::Vec2& _tangentLeft, const ei::Vec2& _tangentRight)
	{
		int idx = 0;
		while(idx < (int)m_handles.size() && m_handles[idx].domainPos.x < _pos.x) ++idx;
		Vec2 posClamped = min(max(_pos, Vec2{m_xDomain.x, m_yDomain.x}), Vec2{m_xDomain.y, m_yDomain.y});
		// While the following forces the endpoints to the correct places it makes problems with sequential additions
		//if(idx == 0) posClamped.x = m_xDomain.x;
		//else if(idx == (int)m_handles.size()) posClamped.x = m_xDomain.y;
		const Vec2 posScreen = posClamped * m_domainToScreen + m_screenOffset;
		const Vec2 tL = (posClamped + _tangentLeft) * m_domainToScreen + m_screenOffset;
		const Vec2 tR = (posClamped + _tangentRight) * m_domainToScreen + m_screenOffset;
		const bool locked = ei::abs(cross(_tangentLeft, _tangentRight)) < 1e-5f;
		m_handles.emplace(m_handles.begin() + idx, Handle{posClamped, posScreen, _tangentLeft, _tangentRight, tL, tR, locked});
		if(idx > 0)
			limitHdl(idx-1, false, true);
		limitHdl(idx, true, true);
		if(idx+1 < (int)m_handles.size())
			limitHdl(idx+1, true, false);
		if(m_onNewHandle)
			m_onNewHandle(idx, posClamped, m_handles[idx].tangentLeft, m_handles[idx].tangentRight);
		return idx;
	}


	void CurveEdit::deleteHandle(int _idx)
	{
		m_selectedHdl = -1;
		if(_idx >= 0 && _idx < (int)m_handles.size())
		{
			m_handles.erase(m_handles.begin() + _idx);
			if(m_onDeleteHandle)
				m_onDeleteHandle(_idx);
		}
	}


	void CurveEdit::clear()
	{
		if(m_onDeleteHandle)
			for(size_t i = 0; i < m_handles.size(); ++i)
				m_onDeleteHandle((int)i);
		m_handles.clear();
	}


	void CurveEdit::onExtentChanged()
	{
		recomputeSpaceConversions();
		for(size_t i = 0; i < m_handles.size(); ++i)
			recomputeScreenPos((int)i);
	}

	void CurveEdit::recomputeSpaceConversions()
	{
		m_screenToDomain = Vec2{m_xRange.y - m_xRange.x, m_yRange.y - m_yRange.x} / Vec2{width(), height()};
		m_domainToScreen = Vec2{width(), height()} / Vec2{m_xRange.y - m_xRange.x, m_yRange.y - m_yRange.x};
		m_screenOffset = position() - Vec2{m_xRange.x, m_yRange.x} * m_domainToScreen;
		m_domainOffset = Vec2{m_xRange.x, m_yRange.x} - position() * m_screenToDomain;
	}

	void CurveEdit::recomputeScreenPos(int hdl, bool pos, bool left, bool right)
	{
		if(pos) m_handles[hdl].screenPos = m_handles[hdl].domainPos * m_domainToScreen + m_screenOffset;
		if(left) m_handles[hdl].screenHdlLeft = (m_handles[hdl].domainPos + m_handles[hdl].tangentLeft) * m_domainToScreen + m_screenOffset;
		if(right) m_handles[hdl].screenHdlRight = (m_handles[hdl].domainPos + m_handles[hdl].tangentRight) * m_domainToScreen + m_screenOffset;
	}

	void CurveEdit::limitHdl(int hdl, bool left, bool right)
	{
		if(left && (hdl > 0))
		{
			const float maxX = m_handles[hdl-1].domainPos.x - m_handles[hdl].domainPos.x;
			const float maxY = m_handles[hdl].tangentLeft.y > 0.0f ? m_yDomain.y - m_handles[hdl].domainPos.y : m_yDomain.x - m_handles[hdl].domainPos.y;
			const float scale = ei::min(m_handles[hdl].tangentLeft.x == 0.0f ? 1.0f : maxX / m_handles[hdl].tangentLeft.x,
										m_handles[hdl].tangentLeft.y == 0.0f ? 1.0f : maxY / m_handles[hdl].tangentLeft.y);
			if(scale < 1.0f)
			{
				m_handles[hdl].tangentLeft *= scale;
				recomputeScreenPos(hdl, false, true, false);
			}
		}
		if(right && ((hdl+1) < (int)m_handles.size()))
		{
			const float maxX = m_handles[hdl+1].domainPos.x - m_handles[hdl].domainPos.x;
			const float maxY = m_handles[hdl].tangentRight.y > 0.0f ? m_yDomain.y - m_handles[hdl].domainPos.y : m_yDomain.x - m_handles[hdl].domainPos.y;
			const float scale = ei::min(m_handles[hdl].tangentRight.x == 0.0f ? 1.0f : maxX / m_handles[hdl].tangentRight.x,
										m_handles[hdl].tangentRight.y == 0.0f ? 1.0f : maxY / m_handles[hdl].tangentRight.y);
			if(scale < 1.0f)
			{
				m_handles[hdl].tangentRight *= scale;
				recomputeScreenPos(hdl, false, false, true);
			}
		}
	}

}} // namespace ca::gui
