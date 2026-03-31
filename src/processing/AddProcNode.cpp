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
#include "AddProcNode.hpp"

act::proc::AddProcNode::AddProcNode() : ProcNodeBase("Add") {
	m_drawSize = ci::ivec2(200, 200);
	m_secondary = 0.0f;

	createNumberInput("primary", [&](number val) { m_sumPort->send(val + m_secondary); });
	createNumberInput("secondary", [&](number val) { m_secondary = val; });

	m_sumPort = createNumberOutput("sum");
}

act::proc::AddProcNode::~AddProcNode() {
}

void act::proc::AddProcNode::setup(act::room::RoomManagers roomMgrs) {
}

void act::proc::AddProcNode::update() {
}

void act::proc::AddProcNode::draw() {
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

ci::Json act::proc::AddProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["secondary"] = m_secondary;
	return json;
}

void act::proc::AddProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "secondary", m_secondary);
}
