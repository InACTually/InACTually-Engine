
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021â€“2025 Lars Engeln, Fabian TÃ¶pfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022-2025

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "DMXDimmerProcNode.hpp"

act::proc::DMXDimmerProcNode::DMXDimmerProcNode() : ProcNodeBase("DMXDimmer") {
	m_drawSize = glm::ivec2(200, 200);

	m_dim = 1.0f;

	m_dimPort = createNumberOutput("dimmer");

	auto triggerInput = createBoolInput("trigger", [&](bool triggered) {
		if (triggered)
			m_dimPort->send(m_dim);
	});

	auto dimmerInput = createNumberInput("dimmer", [&](float dimmer) {
		m_dim = dimmer;
		m_dimPort->send(m_dim);
		if (m_dimmer)
			m_dimmer->setDimmer(m_dim);
	});
}

void act::proc::DMXDimmerProcNode::setup(act::room::RoomManagersRef roomMgrs) {
	m_dmxMgr = roomMgrs->dmxMgr;
}

act::proc::DMXDimmerProcNode::~DMXDimmerProcNode() {
}

void act::proc::DMXDimmerProcNode::update() {
	//m_dimPort->send(m_dim);
}

void act::proc::DMXDimmerProcNode::draw() {
	beginNodeDraw();

	bool sliderUsed = false;

	ImGui::SetNextItemWidth(200);
	if (ImGui::SliderFloat("", &m_dim, 0.0f, 1.0f)) {
		sliderUsed = true;
		m_dimPort->send(m_dim);
		if (m_dimmer)
			m_dimmer->setDimmer(m_dim);
	}

	if (m_dmxMgr->getFixtureNames().empty())
		ImGui::Text("No DMX devices have been set up.");
	else {
		ImGui::SetNextItemWidth(m_drawSize.x - ImGui::CalcTextSize("Dimmer").x);
		if (ImGui::Combo("Dimmer", &m_selectedDimmer, m_dmxMgr->getFixtureNames())) {
			m_dimmer = m_dmxMgr->getDimmerByIndex(m_selectedDimmer);
		}
	}

	preventDrag(sliderUsed);

	endNodeDraw();
}

ci::Json act::proc::DMXDimmerProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["dim"] = m_dim;
	json["selectedDimmer"] = m_selectedDimmer;
	return json;
}

void act::proc::DMXDimmerProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "dim", m_dim);
	util::setValueFromJson(json, "selectedDimmer", m_selectedDimmer);
	m_dimmer = m_dmxMgr->getDimmerByIndex(m_selectedDimmer);
	if(m_dimmer)
		m_dimmer->setDimmer(m_dim);

	m_dimPort->send(m_dim);
}
