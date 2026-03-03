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
#include "Vector2ProcNode.hpp"

act::proc::Vector2ProcNode::Vector2ProcNode() : ProcNodeBase("Vector2") {
	m_drawSize = ci::ivec2(200, 200);
	m_vector = ci::vec2(0);

	m_outPort = createVec2Output("out");
}

act::proc::Vector2ProcNode::~Vector2ProcNode() {
}

void act::proc::Vector2ProcNode::setup(act::room::RoomManagers roomMgrs) {
}

void act::proc::Vector2ProcNode::update() {
}

void act::proc::Vector2ProcNode::draw() {
	beginNodeDraw();

	bool prvntDrag = false;

	ImGui::SetNextItemWidth(m_drawSize.x);
	if (ImGui::DragFloat2("vector", &m_vector)) {
		prvntDrag = true;
		m_outPort->send(m_vector);
	}

	preventDrag(prvntDrag);

	endNodeDraw();
}

ci::Json act::proc::Vector2ProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["vector"] = util::valueToJson(m_vector);
	return json;
}

void act::proc::Vector2ProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "vector", m_vector);
}
