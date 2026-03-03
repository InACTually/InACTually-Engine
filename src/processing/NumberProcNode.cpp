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
#include "NumberProcNode.hpp"

act::proc::NumberProcNode::NumberProcNode() : ProcNodeBase("Number") {
	m_drawSize = ci::ivec2(200, 200);
	m_number = 0.0f;

	m_outPort = createNumberOutput("out");
}

act::proc::NumberProcNode::~NumberProcNode() {
}

void act::proc::NumberProcNode::setup(act::room::RoomManagers roomMgrs) {
}

void act::proc::NumberProcNode::update() {
}

void act::proc::NumberProcNode::draw() {
	beginNodeDraw();

	bool prvntDrag = false;

	ImGui::SetNextItemWidth(m_drawSize.x);
	if (ImGui::DragFloat("number", &m_number)) {
		prvntDrag = true;
		m_outPort->send(m_number);
	}

	preventDrag(prvntDrag);

	endNodeDraw();
}

ci::Json act::proc::NumberProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["number"] = m_number;
	return json;
}

void act::proc::NumberProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "number", m_number);
}
