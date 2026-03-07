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
#include "StringProcNode.hpp"

act::proc::StringProcNode::StringProcNode() : ProcNodeBase("String") {
	m_drawSize = ci::ivec2(200, 200);
	m_text = "";

	m_outPort = createTextOutput("out");
}

act::proc::StringProcNode::~StringProcNode() {
}

void act::proc::StringProcNode::setup(act::room::RoomManagers roomMgrs) {
}

void act::proc::StringProcNode::update() {
}

void act::proc::StringProcNode::draw() {
	beginNodeDraw();

	ImGui::SetNextItemWidth(m_drawSize.x);
	if (ImGui::InputText("text", &m_text, ImGuiInputTextFlags_EnterReturnsTrue)) {
		m_outPort->send(m_text);
	}

	endNodeDraw();
}

ci::Json act::proc::StringProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["text"] = m_text;
	return json;
}

void act::proc::StringProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "text", m_text);
}
