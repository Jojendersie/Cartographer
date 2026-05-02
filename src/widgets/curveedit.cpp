#include "ca/gui/widgets/curveedit.hpp"
#include "ca/gui/guimanager.hpp"
#include "ca/gui/rendering/theme.hpp"
#include "ca/gui/backend/renderbackend.hpp"
#include <ca/pa/log.hpp>

using namespace ei;

namespace ca { namespace gui {

	CurveEdit::CurveEdit() :
		m_flags {},
		m_curve { nullptr },
		m_backgroundColor { 0.0f },
		m_gridColor { 0.1f },
		m_curveColor { 0.1f, 0.7f, 0.1f, 1.0f },
		m_gridSpace { 0.1f },
		m_xDomain {0.0f, 1.0f },
		m_yDomain {0.0f, 1.0f },
		m_xRange {-0.1f, 1.1f },
		m_yRange {-0.1f, 1.1f },
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
		Rect2D domainFrame {
			position() + Vec2{round((m_xDomain.x - m_xRange.x) * m_domainToScreen.x), round((m_yDomain.x - m_yRange.x) * m_domainToScreen.y)},
			position() + Vec2{round((m_xDomain.y - m_xRange.x) * m_domainToScreen.x), round((m_yDomain.y - m_yRange.x) * m_domainToScreen.y)}+1.0f
		};
		if(m_backgroundColor.a > 0.0f)
		{
			GUIManager::theme().drawBackgroundArea(rectangle(), m_backgroundColor.a, Vec3(m_backgroundColor));
			// Make the domain background different
			GUIManager::theme().drawBackgroundArea(domainFrame, m_backgroundColor.a, Vec3(m_backgroundColor) * 0.5f);
		}

		if(m_gridColor.a > 0.0f && m_gridSpace.x > 0.0f)
		{
			const int numTicks = ei::floor((m_xDomain.y - m_xDomain.x) / m_gridSpace.x);
			const float tickSpaceScreen = m_gridSpace.x * m_domainToScreen.x;
			for (int x = 0; x <= numTicks; ++x)
			{
				const float screenX = round(domainFrame.min.x + x * tickSpaceScreen)+0.5f;
				Vec2 pos[2] = {
					Vec2{screenX, domainFrame.min.y},
					Vec2{screenX, domainFrame.max.y}
				};
				GUIManager::theme().drawLine(pos, 2, m_gridColor, m_gridColor);

				// Draw label
				char buf[16];
				const float value = x * m_gridSpace.x + m_xDomain.x;
				std::snprintf(buf, 16, "%.*f", m_labelPrecision.x, value);
				GUIManager::theme().drawText(Vec2{screenX, domainFrame.min.y}, buf, 1.0f, false, ei::Vec4{-1.0f}, 0.5f, 1.0f);
			}
		}

		if(m_gridColor.a > 0.0f && m_gridSpace.y > 0.0f)
		{
			const int numTicks = ei::floor((m_yDomain.y - m_yDomain.x) / m_gridSpace.y);
			const float tickSpaceScreen = m_gridSpace.y * m_domainToScreen.y;
			for (int y = 0; y <= numTicks; ++y)
			{
				const float screenY = round(domainFrame.min.y + y * tickSpaceScreen)+0.5f;
				Vec2 pos[2] = {
					Vec2{domainFrame.min.x, screenY},
					Vec2{domainFrame.max.x, screenY}
				};
				GUIManager::theme().drawLine(pos, 2, m_gridColor, m_gridColor);

				// Draw label
				char buf[16];
				const float value = y * m_gridSpace.y + m_yDomain.x;
				std::snprintf(buf, 16, "%.*f", m_labelPrecision.y, value);
				GUIManager::theme().drawText(Vec2{domainFrame.min.x, screenY}, buf, 1.0f, false, ei::Vec4{-1.0f}, 1.0f, 0.5f);
			}
		}

		// Plot the curve
		constexpr int RES = 24;
		Vec2 buf[RES];
		for(size_t i = 0; i < m_handles.size()-1; ++i)
		{
			// Draw the curve from the current to the next vertex.
			buf[0] = m_handles[i].screenPos;
			buf[RES-1] = m_handles[i+1].screenPos;
			for(int j = 1; j < RES-1; ++j)
			{
				buf[j].x = lerp(buf[0].x, buf[RES-1].x, j / float(RES-1));
				const float domainX = buf[j].x * m_screenToDomain.x + m_domainOffset.x;
				buf[j].y = m_curve->getValue(i, domainX) * m_domainToScreen.y + m_screenOffset.y;
			}
			GUIManager::theme().drawLine(buf, RES, m_curveColor, m_curveColor);

			if (m_flags & SHOW_TANGENTS) 
			{
				buf[0] = m_handles[i].screenHdlLeft;
				buf[1] = m_handles[i].screenPos;
				buf[2] = m_handles[i].screenHdlRight;
				// Draw the two tangent vectors in one go. On either end we only need one tangent.
				const bool boundaryVertex = i == 0 || i == m_handles.size()-1;
				GUIManager::theme().drawLine(buf + (i == 0 ? 1 : 0), boundaryVertex ? 2 : 3, m_curveColor, m_curveColor);
			}
		}

		// Need a new layer for the handles, otherwise the lines overlap.
		GUIManager::pushClipRegion(rectangle());
		for(size_t i = 0; i < m_handles.size(); ++i)
		{
			if(m_handles[i].tangentsLocked)
				GUIManager::theme().drawNodeHandle(m_handles[i].screenPos, 3.5f, Vec3{m_curveColor});
			else
			{
				GUIManager::theme().drawArrowButton(Rect2D{m_handles[i].screenPos-3.5f, m_handles[i].screenPos+3.5f}, SIDE::TOP, false); // TODO: color
			}
			if (m_flags & SHOW_TANGENTS)
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
		const ei::Vec2 oldMarginX = m_xRange - m_xDomain;
		const ei::Vec2 oldMarginY = m_yRange - m_yDomain;
		m_xDomain = _xRange;
		m_yDomain = _yRange;
		m_xRange = m_xDomain + oldMarginX;
		m_yRange = m_yDomain + oldMarginY;
		recomputeSpaceConversions();
		scale *= m_domainToScreen;
		translation += m_screenOffset;
		for(size_t i = 0; i < m_handles.size(); ++i)
		{
			m_handles[i].screenPos = m_handles[i].screenPos * scale + translation;
			m_handles[i].screenHdlLeft = m_handles[i].screenHdlLeft * scale + translation;
			m_handles[i].screenHdlRight = m_handles[i].screenHdlRight * scale + translation;
		}
	}


	void CurveEdit::setDomainMargin(float _xNeg, float _xPos, float _yNeg, float _yPos)
	{
		m_xRange = m_xDomain + Vec2{-max(0.0f,_xNeg), max(0.0f,_xPos)};
		m_yRange = m_yDomain + Vec2{-max(0.0f,_yNeg), max(0.0f,_yPos)};
		recomputeSpaceConversions();
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
					if (m_curve) {
						m_curve->onPositionChanged(m_selectedHdl, domainPos);
						updateHandles();
					}
					else ca::pa::logError("[CurveEdit::processInput] Cannot set position.");
				}
			}
			else // A handle is selected
			{
				const int idx = m_selectedSubHdl / 2;
				const bool selectedRight = m_selectedSubHdl & 1;
				if (m_curve)
				{
					const Vec2 tangent = (_mouseState.position - m_handles[idx].screenPos) * m_screenToDomain;
					m_curve->onTangentChanged(idx, tangent, !selectedRight);
					updateHandles();
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
			if ((m_flags & TANGENT_EDITING) == TANGENT_EDITING)
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
		if((m_flags & TOGGLE_NODE_TYPE) && _mouseState.btnDblClicked(0) && clickedHdl != -1)
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
		else if((m_flags & ADD_NODES) && _mouseState.btnDblClicked(0))
		{
			if(domainPos.x >= m_xDomain.x && domainPos.x <= m_xDomain.y
				&& domainPos.y >= m_yDomain.x && domainPos.y <= m_yDomain.y)
			{
				int idx = 0;
				while(idx < (int)m_handles.size() && m_handles[idx].screenPos.x < _mouseState.position.x) ++idx;
				bool added = false;
				if (m_curve)
					added = m_curve->onNewHandle(idx);
				if (added)
				{
					m_curve->onPositionChanged(idx, domainPos);
					m_handles.emplace(m_handles.begin() + idx, Handle{_mouseState.position, {}, {}, true});
				}
				updateHandles();
			}
		}
		else if((m_flags & REMOVE_NODES) && _mouseState.btnDblClicked(1) && clickedHdl != -1)
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
		m_handles.insert(m_handles.begin() + start, _num, Handle{{},{},{},true});
		updateHandles();
	}


	void CurveEdit::deleteHandle(int _idx)
	{
		m_selectedHdl = -1;
		if(_idx >= 0 && _idx < (int)m_handles.size())
		{
			m_handles.erase(m_handles.begin() + _idx);
			if(m_curve)
				m_curve->onDeleteHandle(_idx);
		}
	}


	void CurveEdit::clear()
	{
		if(m_flags & REMOVE_NODES && m_curve)
			for(size_t i = 0; i < m_handles.size(); ++i)
				m_curve->onDeleteHandle((int)i);
		m_handles.clear();
	}


	void CurveEdit::onExtentChanged()
	{
		recomputeSpaceConversions();
		updateHandles();
	}

	void CurveEdit::recomputeSpaceConversions()
	{
		m_screenToDomain = Vec2{m_xRange.y - m_xRange.x, m_yRange.y - m_yRange.x} / Vec2{width(), height()};
		m_domainToScreen = Vec2{width(), height()} / Vec2{m_xRange.y - m_xRange.x, m_yRange.y - m_yRange.x};
		m_screenOffset = position() - Vec2{m_xRange.x, m_yRange.x} * m_domainToScreen;
		m_domainOffset = Vec2{m_xRange.x, m_yRange.x} - position() * m_screenToDomain;
	}


	void CurveEdit::updateHandles()
	{
		if (!m_curve)
			return;
		for (size_t i = 0; i < m_handles.size(); ++i)
		{
			m_handles[i].screenPos = Vec2 { round(m_curve->getPosition(i) * m_domainToScreen + m_screenOffset) };
			if (m_flags & SHOW_TANGENTS)
			{
				m_handles[i].screenHdlLeft = m_handles[i].screenPos +  Vec2 { round(m_curve->getTangent(i, true) * m_domainToScreen) };
				m_handles[i].screenHdlRight = m_handles[i].screenPos + Vec2 { round(m_curve->getTangent(i, false) * m_domainToScreen) };
			}
		}
	}

}} // namespace ca::gui
