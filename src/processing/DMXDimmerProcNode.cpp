
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021�2025 Lars Engeln, Fabian T�pfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "DMXDimmerProcNode.hpp"

act::proc::DMXDimmerProcNode::DMXDimmerProcNode() : ProcNodeBase("DMXDimmer") {
	m_drawSize = ci::ivec2(200, 200);

	m_dim = 1.0f;

	m_dimPort = OutputPort<float> ::create(PT_NUMBER, "dimmer");
	m_outputPorts.push_back(m_dimPort);

	auto triggerInput = InputPort<bool>::create(PT_BOOL, "trigger", [&](bool triggered) {
		if (triggered)
			m_dimPort->send(m_dim);
		});
	m_inputPorts.push_back(triggerInput);
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
	}

	preventDrag(sliderUsed);

	endNodeDraw();
}

ci::Json act::proc::DMXDimmerProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["dim"] = m_dim;
	return json;
}

void act::proc::DMXDimmerProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "dim", m_dim);
}
