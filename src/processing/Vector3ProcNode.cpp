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
#include "Vector3ProcNode.hpp"

act::proc::Vector3ProcNode::Vector3ProcNode() : ProcNodeBase("Vector3") {
	m_drawSize = ci::ivec2(200, 200);
	m_vector = ci::vec3(0);

	m_outPort = createVec3Output("out");
}

act::proc::Vector3ProcNode::~Vector3ProcNode() {
}

void act::proc::Vector3ProcNode::setup(act::room::RoomManagers roomMgrs) {
}

void act::proc::Vector3ProcNode::update() {
}

void act::proc::Vector3ProcNode::draw() {
	beginNodeDraw();

	bool prvntDrag = false;

	ImGui::SetNextItemWidth(m_drawSize.x);
	if (ImGui::DragFloat3("vector", &m_vector)) {
		prvntDrag = true;
		m_outPort->send(m_vector);
	}

	preventDrag(prvntDrag);

	endNodeDraw();
}

ci::Json act::proc::Vector3ProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["vector"] = util::valueToJson(m_vector);
	return json;
}

void act::proc::Vector3ProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "vector", m_vector);
}
