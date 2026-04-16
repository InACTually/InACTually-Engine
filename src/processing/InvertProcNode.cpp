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
#include "InvertProcNode.hpp"

act::proc::InvertProcNode::InvertProcNode() : ProcNodeBase("Invert") {
	m_drawSize = ci::ivec2(200, 200);
	m_secondary = 1.0f;

	createNumberInput("primary", [&](number val) { m_invertedPort->send(m_secondary - val); });
	createNumberInput("secondary", [&](number val) { m_secondary = val; });

	m_invertedPort = createNumberOutput("inverted");
}

act::proc::InvertProcNode::~InvertProcNode() {
}

void act::proc::InvertProcNode::setup(act::room::RoomManagers roomMgrs) {
}

void act::proc::InvertProcNode::update() {
}

void act::proc::InvertProcNode::draw() {
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

ci::Json act::proc::InvertProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["secondary"] = m_secondary;
	return json;
}

void act::proc::InvertProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "secondary", m_secondary);
}
