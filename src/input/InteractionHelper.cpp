
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/gl/Texture.h"

#include "cinder/audio/audio.h"

#include "InteractionHelper.hpp"
#include "..\..\include\input\InteractionHelper.hpp"
#include "imgui/imgui.h"


using namespace ci; 
using namespace ci::app;
using namespace std;

using namespace act;
using namespace ia;

InteractionHelper::InteractionHelper()
{
	m_mousePosNorm = vec2(0.0f);
	m_wheelIncrement = 0.0f;
	m_isDragging = false;
}

InteractionHelper::~InteractionHelper()
{
}

input::MouseListener*	InteractionHelper::getMouseListener()	{ return (MouseListener*)	this; }
//input::KeyListener*		InteractionHelper::getKeyListener()		{ return (KeyListener*)		this; }
//input::TouchListener*	InteractionHelper::getTouchListener()	{ return (TouchListener*)	this; }

void act::ia::InteractionHelper::evaluate() {
	evaluateMouse();
}

bool act::ia::InteractionHelper::evaluateMouse() {
	m_mousePos = ImGui::GetMousePos();
	ImVec2 vMin = ImGui::GetItemRectMin();
	ImVec2 vMax = ImGui::GetItemRectMax();
	m_mousePos.x = m_mousePos.x - vMin.x;
	m_mousePos.y = m_mousePos.y - vMin.y;
	m_mousePosNorm.x = (float)m_mousePos.x / (float)(vMax.x - vMin.x);
	m_mousePosNorm.y = (float)m_mousePos.y / (float)(vMax.y - vMin.y);


	if (evaluateMouseButton(MB_BTNLEFT))
		return true;
	if (evaluateMouseButton(MB_BTNRIGHT))
		return true;
	if (evaluateMouseButton(MB_BTNMIDDLE))
		return true;

	// else
	ci::app::MouseEvent evt = ci::app::MouseEvent();
	evt.setPos(vec2(m_mousePos.x, m_mousePos.y));
	this->onMouseMove(evt);

	return false;
}

bool act::ia::InteractionHelper::evaluateMouseButton(MouseButton btn)
{
	unsigned int modifier = 0;

	switch (btn) {
		case MB_BTNRIGHT:
			modifier = ci::app::MouseEvent::RIGHT_DOWN;
			break;
		case MB_BTNMIDDLE:
			modifier = ci::app::MouseEvent::MIDDLE_DOWN;
			break;
		case MB_BTNLEFT:
		default:
			modifier = ci::app::MouseEvent::LEFT_DOWN;
			break;
	}

	ci::app::MouseEvent evt = ci::app::MouseEvent(getWindow(), 0, m_mousePos.x, m_mousePos.y, modifier, m_wheelIncrement, modifier);
	evt.setPos(ivec2(m_mousePos.x, m_mousePos.y));
	if (ImGui::IsMouseDown(btn)) {
		if (ImGui::IsMouseDragging(btn) && m_isDragging) {
			this->onMouseDrag(evt);
			return true;
		}
		if (m_mousePosNorm.x >= 0.0f && m_mousePosNorm.y >= 0.0f && m_mousePosNorm.x < 1.0f && m_mousePosNorm.y < 1.0f) {
			m_isDragging = true;
			this->onMouseDown(evt);
		}
		return true;
	}
	if (ImGui::IsMouseReleased(btn)) {
		m_isDragging = false;
		this->onMouseUp(evt);
		return true;
	}
	if (ImGui::GetIO().MouseWheel) {
		m_wheelIncrement = ImGui::GetIO().MouseWheel;
		evt = ci::app::MouseEvent(getWindow(), 0, m_mousePos.x, m_mousePos.y, modifier, m_wheelIncrement, modifier);
		this->onMouseWheel(evt);
		return true;
	}
	

	return false;
}
