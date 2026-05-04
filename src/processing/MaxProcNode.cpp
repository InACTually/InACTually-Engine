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
#include "MaxProcNode.hpp"

act::proc::MaxProcNode::MaxProcNode() : ProcNodeBase("Max") {
	m_drawSize = ci::ivec2(200, 200);
	m_secondary = 0.0f;

	createNumberInput("primary", [&](number val) { m_maxPort->send(std::max(val, m_secondary)); });
	createNumberInput("secondary", [&](number val) { m_secondary = val; });

	m_maxPort = createNumberOutput("max");
}

act::proc::MaxProcNode::~MaxProcNode() {
}

void act::proc::MaxProcNode::setup(act::room::RoomManagers roomMgrs) {
}

void act::proc::MaxProcNode::update() {
}

void act::proc::MaxProcNode::draw() {
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

ci::Json act::proc::MaxProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["secondary"] = m_secondary;
	return json;
}

void act::proc::MaxProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "secondary", m_secondary);
}
