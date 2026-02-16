#include "ca/gui/widgets/curveedit.hpp"
#include "ca/gui/guimanager.hpp"
#include "ca/gui/rendering/theme.hpp"
#include "ca/gui/backend/renderbackend.hpp"
#include <ca/pa/log.hpp>

using namespace ei;

namespace ca { namespace gui {

	CurveEdit::CurveEdit() :
		m_onNewHandle { nullptr },
		m_onDeleteHandle { nullptr },
		m_getTangent { nullptr },
		m_getPosition { nullptr },
		m_onPositionChanged { nullptr},
		m_onTangentChanged { nullptr },
		m_backgroundColor { 0.0f },
		m_gridColor { 0.1f },
		m_curveColor { 0.1f, 0.7f, 0.1f, 1.0f },
		m_gridSpace { 0.1f },
		m_xDomain {0.0f, 1.0f },
		m_yDomain {0.0f, 1.0f },
		m_xRange {-0.1f, 1.1f },
		m_yRange {-0.1f, 1.1f },
		m_mode { Mode::BEZIER },
		m_periodic { false },
		m_tangentLength { 16.0f },
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
			t*t*(t-1) };
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
		else if(m_mode == Mode::SMOOTH || m_mode == Mode::CUBIC_HERMITE)
		{
			constexpr int RES = 24;
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
				hemite_spline_coeffs(10.0f / (RES-1)),
				hemite_spline_coeffs(11.0f / (RES-1)),
				hemite_spline_coeffs(12.0f / (RES-1)),
				hemite_spline_coeffs(13.0f / (RES-1)),
				hemite_spline_coeffs(14.0f / (RES-1)),
				hemite_spline_coeffs(15.0f / (RES-1)),
				hemite_spline_coeffs(16.0f / (RES-1)),
				hemite_spline_coeffs(17.0f / (RES-1)),
				hemite_spline_coeffs(18.0f / (RES-1)),
				hemite_spline_coeffs(19.0f / (RES-1)),
				hemite_spline_coeffs(20.0f / (RES-1)),
				hemite_spline_coeffs(21.0f / (RES-1)),
				hemite_spline_coeffs(22.0f / (RES-1))
			};
			for(size_t i = 0; i < m_handles.size(); ++i)
			{
				buf[1] = m_handles[i].screenPos;
				// Draw the two tangent vectors in one go. On either end we only need one tangent.
				if (m_mode == Mode::CUBIC_HERMITE)
				{
					buf[0] = m_handles[i].screenHdlLeft;
					buf[2] = m_handles[i].screenHdlRight;
					GUIManager::theme().drawLine(buf + (i == 0 ? 1 : 0), 3 - (i == 0 || i == m_handles.size()-1 ? 1 : 0), m_curveColor, m_curveColor);
				}
				// Draw the actual curve back to the previous node.
				if(i > 0)
				{
					const float lSlopeR = m_handles[i-1].screenTangentRight.y / m_handles[i-1].screenTangentRight.x * (m_handles[i].screenPos.x - m_handles[i-1].screenPos.x);
					const float rSlopeL = m_handles[i].screenTangentLeft.y / m_handles[i].screenTangentLeft.x * (m_handles[i].screenPos.x - m_handles[i-1].screenPos.x);
					buf[0] = m_handles[i-1].screenPos;
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
			constexpr int RES = 24;
			Vec2 buf[RES];
			for(size_t i = 0; i < m_handles.size(); ++i)
			{
				buf[0] = m_handles[i].screenHdlLeft;
				buf[1] = m_handles[i].screenPos;
				buf[2] = m_handles[i].screenHdlRight;
				// Draw the two tangent vectors in one go. On either end we only need one tangent.
				GUIManager::theme().drawLine(buf + (i == 0 ? 1 : 0), 3 - (i == 0 || i == m_handles.size()-1 ? 1 : 0), m_curveColor, m_curveColor);
				// Draw the actual curve back to the previous node.
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
			if(m_mode != Mode::LINEAR && m_mode != Mode::SMOOTH)
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
		// Compute a screenspace -> screenspace transform for the existing nodes.
		ei::Vec2 translation = -m_screenOffset;
		ei::Vec2 scale = m_screenToDomain;
		m_xDomain = _xRange;
		m_yDomain = _yRange;
		recomputeSpaceConversions();
		scale *= m_domainToScreen;
		translation += m_screenOffset;
		for(size_t i = 0; i < m_handles.size(); ++i)
		{
			m_handles[i].screenPos = m_handles[i].screenPos * scale + translation;
			m_handles[i].screenTangentLeft *= scale;
			m_handles[i].screenTangentRight *= scale;
			m_handles[i].screenHdlLeft = m_handles[i].screenPos + m_handles[i].screenTangentLeft;
			m_handles[i].screenHdlRight = m_handles[i].screenPos + m_handles[i].screenTangentRight;
		}
	}


	void CurveEdit::setDomainMargin(float _xNeg, float _xPos, float _yNeg, float _yPos)
	{
		m_xRange = m_xDomain + Vec2{-max(0.0f,_xNeg), max(0.0f,_xPos)};
		m_yRange = m_yDomain + Vec2{-max(0.0f,_yNeg), max(0.0f,_yPos)};
		recomputeSpaceConversions();
	}


	void CurveEdit::setMode(Mode _mode, bool _periodic)
	{
		m_periodic = _periodic;
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
				m_selectedSubHdl = m_selectedHdl = -1;
			}
			// Hdl and SubHdl are exclusive, so if one is defined, the other is not.
			else if(m_selectedHdl != -1)
			{
				if (_mouseState.position != m_handles[m_selectedHdl].screenPos) // Changed?
				{
					if (m_onPositionChanged) {
						const ei::IVec2 interval = m_onPositionChanged(m_selectedHdl, domainPos);
						updateHandles(interval);
					}
					else ca::pa::logError("[CurveEdit::processInput] Cannot set position.");
				}
			}
			else // A handle is selected
			{
				const int idx = m_selectedSubHdl / 2;
				const bool selectedRight = m_selectedSubHdl & 1;
				if (m_onTangentChanged)
				{
					const Vec2 tangent = (_mouseState.position - m_handles[idx].screenPos) * m_screenToDomain;
					const ei::IVec2 interval = m_onTangentChanged(idx, tangent, !selectedRight);
					updateHandles(interval);
				}
			}
		}

		// Check if the mouse is over one of the handles
		int clickedHdl = -1;
		int clickedSubHdl = -1;
		for(int i = 0; i < (int)m_handles.size(); ++i)
		{
			if(lensq(m_handles[i].screenPos - _mouseState.position) < 15.0f)
				clickedHdl = i;
			if(m_mode != Mode::LINEAR && m_mode != Mode::SMOOTH)
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
			/*if(m_handles[clickedHdl].tangentsLocked)
			{
				const Vec2 newDir = m_handles[clickedHdl].screenTangentRight - m_handles[clickedHdl].screenTangentLeft;
				const float newDirLenSq = 1e-10f + lensq(newDir);
				m_handles[clickedHdl].screenTangentLeft = -newDir * sqrtf(lensq(m_handles[clickedHdl].screenTangentLeft) / newDirLenSq);
				m_handles[clickedHdl].screenTangentRight = newDir * sqrtf(lensq(m_handles[clickedHdl].screenTangentRight) / newDirLenSq);
				// TODO: notify the model or ditch the functionality of handle types
				//recomputeScreenPos(clickedHdl, false, true, true);
			}*/
		}
		// Add a new handle on double click
		else if(_mouseState.btnDblClicked(0))
		{
			if(domainPos.x >= m_xDomain.x && domainPos.x <= m_xDomain.y
				&& domainPos.y >= m_yDomain.x && domainPos.y <= m_yDomain.y)
			{
				int idx = 0;
				while(idx < (int)m_handles.size() && m_handles[idx].screenPos.x < _mouseState.position.x) ++idx;
				m_handles.emplace(m_handles.begin() + idx, Handle{_mouseState.position, {}, {}, {}, {}, true});
				IVec2 updateInterval {idx, idx+1};
				if (m_onNewHandle)
					updateInterval = m_onNewHandle(idx, domainPos);
				updateHandles(updateInterval);
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


	void CurveEdit::addHandles(int _idx, int _num)
	{
		const int start = clamp(_idx, 0, (int)m_handles.size());
		m_handles.insert(m_handles.begin() + start, _num, Handle{{},{},{},{},{},true});
		updateHandles({max(0,_idx - 1), min(_idx + _num, (int)m_handles.size())});
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
		updateHandles({0, (int)m_handles.size()});
	}

	void CurveEdit::recomputeSpaceConversions()
	{
		m_screenToDomain = Vec2{m_xRange.y - m_xRange.x, m_yRange.y - m_yRange.x} / Vec2{width(), height()};
		m_domainToScreen = Vec2{width(), height()} / Vec2{m_xRange.y - m_xRange.x, m_yRange.y - m_yRange.x};
		m_screenOffset = position() - Vec2{m_xRange.x, m_yRange.x} * m_domainToScreen;
		m_domainOffset = Vec2{m_xRange.x, m_yRange.x} - position() * m_screenToDomain;
	}


	void CurveEdit::updateHandles(const ei::IVec2& interval)
	{
		const int n = m_handles.size();
		int start = m_periodic ? interval.x : max(interval.x, 0);
		int end = m_periodic ? interval.y : clamp(interval.y, 0, n);
		if (start > end && m_periodic)
			end += m_handles.size();
		for(int i = start; i < end; ++i)
		{
			const int idx = (i + n) % n;
			m_handles[idx].screenPos = Vec2 { round(m_getPosition(idx) * m_domainToScreen + m_screenOffset) };
			m_handles[idx].screenTangentLeft = Vec2 { round(m_getTangent(idx, true) * m_domainToScreen) };
			m_handles[idx].screenTangentRight = Vec2 { round(m_getTangent(idx, false) * m_domainToScreen) };
			if(m_mode == Mode::CUBIC_HERMITE)
			{
				m_handles[idx].screenTangentLeft = normalize(m_handles[idx].screenTangentLeft) * m_tangentLength;
				m_handles[idx].screenTangentRight = normalize(m_handles[idx].screenTangentRight) * m_tangentLength;
			}
			m_handles[idx].screenHdlLeft = m_handles[idx].screenPos + m_handles[idx].screenTangentLeft;
			m_handles[idx].screenHdlRight = m_handles[idx].screenPos + m_handles[idx].screenTangentRight;
		}
	}

}} // namespace ca::gui
