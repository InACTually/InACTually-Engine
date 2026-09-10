
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021â€“2025 Lars Engeln, Fabian TÃ¶pfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022-2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "BodyTrackingProcNode.hpp"

act::proc::BodyTrackingProcNode::BodyTrackingProcNode() : ProcNodeBase("BodyTracking") {

	m_drawSize = glm::ivec2(336, 189);

	m_numberOfBodies = 0;

	m_bodiesOutPort = OutputPort<room::BodyRefList>::create(PT_BODYLIST, "bodies");
	m_outputPorts.push_back(m_bodiesOutPort);
}

act::proc::BodyTrackingProcNode::~BodyTrackingProcNode() {
}

void act::proc::BodyTrackingProcNode::setup(act::room::RoomManagersRef roomMgrs) {
	m_bodyTrackingMgr = roomMgrs->bodyTrackingMgr;

	m_bodiesInPort = InputPort<room::BodyRefList>::create(PT_BODYLIST, "bodies", [&](room::BodyRefList bodies) {
		m_bodies = bodies;
		m_numberOfBodies = m_bodies.size();
		if (m_numberOfBodies > 0)
			m_bodiesOutPort->send(m_bodies);
	});

	m_bodyTrackingMgr->getOutputPort()->connect(m_bodiesInPort);
}

void act::proc::BodyTrackingProcNode::update() {

}

void act::proc::BodyTrackingProcNode::draw() {
	beginNodeDraw();

	ImGui::Text("Bodies in Room: %d", m_numberOfBodies);

	ImGui::NewLine();

	ImGui::Text("Devices:");
	for (auto&& device : m_contributingDevices) {
		ImGui::BulletText("%s", device.c_str());
	}

	endNodeDraw();
}