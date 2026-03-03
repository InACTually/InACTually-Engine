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
#include "BooleanProcNode.hpp"

act::proc::BooleanProcNode::BooleanProcNode() : ProcNodeBase("Boolean") {
	m_drawSize = ci::ivec2(200, 200);
	m_boolean = true;

	m_outPort = createBoolOutput("out");
}

act::proc::BooleanProcNode::~BooleanProcNode() {
}

void act::proc::BooleanProcNode::setup(act::room::RoomManagers roomMgrs) {
}

void act::proc::BooleanProcNode::update() {
}

void act::proc::BooleanProcNode::draw() {
	beginNodeDraw();

	bool prvntDrag = false;

	if (ImGui::Checkbox("boolean", &m_boolean)) {
		prvntDrag = true;
		m_outPort->send(m_boolean);
	}

	preventDrag(prvntDrag);

	endNodeDraw();
}

ci::Json act::proc::BooleanProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["boolean"] = m_boolean;
	return json;
}

void act::proc::BooleanProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "boolean", m_boolean);
}
