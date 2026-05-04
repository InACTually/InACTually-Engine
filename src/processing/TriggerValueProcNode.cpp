/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2026 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "TriggerValueProcNode.hpp"

act::proc::TriggerValueProcNode::TriggerValueProcNode() : ProcNodeBase("TriggerValue") {
	m_drawSize = ci::ivec2(200, 200);
	m_value = 0.0;

	createBoolInput("fire", [&](bool val) {
		m_valuePort->send(m_value);
	});

	m_valuePort = createNumberOutput("value");
}

act::proc::TriggerValueProcNode::~TriggerValueProcNode() {
}

void act::proc::TriggerValueProcNode::setup(act::room::RoomManagers roomMgrs) {
}

void act::proc::TriggerValueProcNode::update() {
}

void act::proc::TriggerValueProcNode::draw() {
	beginNodeDraw();

	ImGui::Text(getName().c_str());

	bool prvntDrag = false;

	ImGui::SetNextItemWidth(m_drawSize.x);
	if (ImGui::DragFloat("value", &m_value)) {
		prvntDrag = true;
	}

	preventDrag(prvntDrag);

	endNodeDraw();
}

ci::Json act::proc::TriggerValueProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["value"] = m_value;
	return json;
}

void act::proc::TriggerValueProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "value", m_value);
}
