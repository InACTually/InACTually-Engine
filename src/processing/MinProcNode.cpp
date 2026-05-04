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
#include "MinProcNode.hpp"

act::proc::MinProcNode::MinProcNode() : ProcNodeBase("Min") {
	m_drawSize = ci::ivec2(200, 200);
	m_secondary = 1.0f;

	createNumberInput("primary", [&](number val) { m_minPort->send(std::min(val, m_secondary)); });
	createNumberInput("secondary", [&](number val) { m_secondary = val; });

	m_minPort = createNumberOutput("min");
}

act::proc::MinProcNode::~MinProcNode() {
}

void act::proc::MinProcNode::setup(act::room::RoomManagers roomMgrs) {
}

void act::proc::MinProcNode::update() {
}

void act::proc::MinProcNode::draw() {
	beginNodeDraw();

	ImGui::Text(getName().c_str());

	bool prvntDrag = false;

	ImGui::SetNextItemWidth(m_drawSize.x);
	if (ImGui::DragFloat("secondary", &m_secondary)) {
		prvntDrag = true;
	}

	preventDrag(prvntDrag);

	endNodeDraw();
}

ci::Json act::proc::MinProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["secondary"] = m_secondary;
	return json;
}

void act::proc::MinProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "secondary", m_secondary);
}
