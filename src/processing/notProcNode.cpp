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
#include "notProcNode.hpp"

act::proc::notProcNode::notProcNode() : ProcNodeBase("not") {
	m_drawSize = ci::ivec2(200, 200);
	
	m_notPort = createBoolOutput("not");

	createBoolInput("bool", [&](bool val) { m_notPort->send(!val);  });
}

act::proc::notProcNode::~notProcNode() {
}

void act::proc::notProcNode::setup(act::room::RoomManagers roomMgrs) {
}

void act::proc::notProcNode::update() {
}

void act::proc::notProcNode::draw() {
	beginNodeDraw();

	ImGui::Text(getName().c_str());


	endNodeDraw();
}

ci::Json act::proc::notProcNode::toParams() {
	ci::Json json = ci::Json::object();
	return json;
}

void act::proc::notProcNode::fromParams(ci::Json json) {
}
