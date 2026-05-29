
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

#include "procpch.hpp"
#include "MovingHeadProcNode.hpp"

act::proc::MovingHeadProcNode::MovingHeadProcNode() : ProcNodeBase("MovingHead", NT_OUTPUT) {
	m_drawSize = ivec2(400, 300);
	
	m_dim = 1.0f;
	m_zoom = 0.0f;
	m_color = Color::white();
	m_lookAt = vec3(0.0f);
	m_isLookingAt = false;

	auto dimmer		= createNumberInput("dimmer",			[&](float dim) { this->onDimmer(dim); });
	auto color		= createColorInput("color",				[&](ci::Color color) { this->onColor(color); });
	auto islooking	= createBoolInput("isLookingAt",		[&](bool isLookingAt) { m_isLookingAt = isLookingAt; });
	auto lookAt		= createVec3Input("lookAt",				[&](vec3 lookAt) { this->onLookAt(lookAt); });
	auto lookInDir	= createVec3Input("lookInDirection",	[&](vec3 lookDir) { 
		if(m_movingHead)
			this->onLookAt(m_movingHead->getPosition() + (ci::normalize(lookDir) * 2.0f));
	});


	if (m_movingHead) {
		m_movingHead->setDimmer(m_dim);
		m_movingHead->setColor(m_color);
		m_movingHead->setZoom(m_zoom);
	}
}

act::proc::MovingHeadProcNode::~MovingHeadProcNode() {
}

void act::proc::MovingHeadProcNode::setup(act::room::RoomManagers roomMgrs) {
	m_dmxMgr = roomMgrs.dmxMgr;
}

void act::proc::MovingHeadProcNode::update() {
	if (m_movingHead) {
		m_movingHead->setIsHighlighted(m_isHovered);
		m_movingHead->setIsShowingDetails(m_isSelected);
	}
}

void act::proc::MovingHeadProcNode::draw() {
	beginNodeDraw();
	
	if (m_dmxMgr->getFixtureNames().empty())
		ImGui::Text("No DMX devices have been set up.");
	else {
		ImGui::SetNextItemWidth(m_drawSize.x - ImGui::CalcTextSize("MovingHead").x);
		if (ImGui::Combo("MovingHead", &m_selectedMovingHead, m_dmxMgr->getFixtureNames())) {
			m_movingHead = m_dmxMgr->getMovingHeadByIndex(m_selectedMovingHead);
		}

		if (m_movingHead) {
			ImGui::PushItemWidth(m_drawSize.x);
			ImGui::Text(m_movingHead->getName().c_str());
			bool sliderUsed = false;
			if (ImGui::SliderFloat("dimmer", &m_dim, 0.0f, 1.0f)) {
				sliderUsed = true;
				onDimmer(m_dim);
			}
			if (m_movingHead->hasZoom()) {
				if (ImGui::SliderFloat("zoom", &m_zoom, 0.0f, 1.0f)) {
					sliderUsed = true;
					onZoom(m_zoom);
				}
			}
			if (ImGui::Checkbox("isLookingAt", &m_isLookingAt)) {
				m_movingHead->isLookingAt(m_isLookingAt);
				if (m_isLookingAt)
					onLookAt(m_lookAt);
			}
			preventDrag(sliderUsed);
			ImGui::PopItemWidth();
		}
	}

	endNodeDraw();
}

void act::proc::MovingHeadProcNode::onDimmer(float dim) {
	m_dim = dim;
	if (m_movingHead)
		m_movingHead->setDimmer(dim);
}

void act::proc::MovingHeadProcNode::onZoom(float zoom)
{
	m_zoom = zoom;
	if (m_movingHead)
		m_movingHead->setZoom(zoom);
}

void act::proc::MovingHeadProcNode::onColor(ci::Color color) {
	ci::app::timeline().apply(&m_color, color, 3, ci::easeInOutSine).updateFn([this]() {
		if (m_movingHead)
			m_movingHead->setColor(m_color);
		});
}

void act::proc::MovingHeadProcNode::onLookAt(vec3 lookAt) {
	m_lookAt = lookAt;
	if (m_movingHead && m_isLookingAt)
		m_movingHead->lookAt(lookAt);
}

ci::Json act::proc::MovingHeadProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["isLookingAt"]			= m_isLookingAt;
	json["dimmer"]				= m_dim;
	json["zoom"]				= m_zoom;
	json["selectedMovingHead"]	= m_selectedMovingHead;
	json["isUpSideDown"]		= m_isUpsideDown;
	return json;
}

void act::proc::MovingHeadProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "isLookingAt", m_isLookingAt);
	util::setValueFromJson(json, "dimmer", m_dim);
	util::setValueFromJson(json, "zoom", m_zoom);
	util::setValueFromJson(json, "selectedMovingHead", m_selectedMovingHead);
	util::setValueFromJson(json, "isUpSideDown", m_isUpsideDown);
	m_movingHead = m_dmxMgr->getMovingHeadByIndex(m_selectedMovingHead);
}
