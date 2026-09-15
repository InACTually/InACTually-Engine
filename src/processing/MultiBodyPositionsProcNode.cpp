
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "MultiBodyPositionsProcNode.hpp"


act::proc::MultiBodyPositionsProcNode::MultiBodyPositionsProcNode() : ProcNodeBase("MultiBodyPositions") {
	m_drawSize = glm::ivec2(250, 250);

	for (int i = 0; i < 4; i++) {
		{
			std::stringstream strstr;
			strstr << "Position " << (i + 1);
			auto positionOutPort = createVec3Output(strstr.str());
			m_positionPorts.push_back(positionOutPort);
		}
		{
			std::stringstream strstr;
			strstr << "Number " << (i + 1);
			auto distanceOutPort = createNumberOutput(strstr.str());
			m_distancePorts.push_back(distanceOutPort);
		}
	}

	auto bodies = InputPort<room::BodyRefList>::create(PT_BODYLIST, "Bodies", [&](room::BodyRefList bodies) { this->onBodies(bodies); });
	m_inputPorts.push_back(bodies);

	m_currentJoint = 0;
}

act::proc::MultiBodyPositionsProcNode::~MultiBodyPositionsProcNode() {
}


void act::proc::MultiBodyPositionsProcNode::setup() {
}

void act::proc::MultiBodyPositionsProcNode::update() {
}

void act::proc::MultiBodyPositionsProcNode::draw() {
	beginNodeDraw();

	ImGui::SetNextItemWidth(m_drawSize.x - ImGui::CalcTextSize("Skeleton").x);

	ImGui::Combo("Joint Selection", &m_currentJoint, m_jointSelection, IM_ARRAYSIZE(m_jointSelection));

	endNodeDraw();
}


void act::proc::MultiBodyPositionsProcNode::onBodies(room::BodyRefList event) {
	if (event.empty() || event.size() == 0)
		return;

	if (event.size() > 4)
		event.resize(4);

	for (int i = 0; i < event.size(); i++) {
		if (!event[i])
			continue;

		float minDistance = 1000.f;

		auto pos = onSkeleton(event[i]);
		for (int j = 0; j < event.size(); j++) {
			if (!event[j] || i == j)
				continue;

			float distance = ci::distance(pos, onSkeleton(event[j]));
			if (distance < minDistance) {
				minDistance = distance;
			}
		}

		m_positionPorts[i]->send(pos);
		m_distancePorts[i]->send(minDistance);
	}
}

glm::vec3 act::proc::MultiBodyPositionsProcNode::onSkeleton(room::BodyRef event) {
	if (m_currentJoint == 2) { // hand
		return event->joints[room::BJT_HAND_RIGHT]->position;
	}
	if (m_currentJoint == 3) { // hand
		return event->joints[room::BJT_HAND_LEFT]->position;
	}
	if (m_currentJoint == 1) { // floor
		float y = std::min(event->joints[room::BJT_FOOT_LEFT]->position.y, event->joints[room::BJT_FOOT_RIGHT]->position.y);
		glm::vec3 pos = event->getPosition();
		pos.y = y;
		return pos;
	}

	// spine-chest
	return event->getPosition();
}


ci::Json act::proc::MultiBodyPositionsProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["selectedJoint"] = m_currentJoint;
	return json;
}

void act::proc::MultiBodyPositionsProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "selectedJoint", m_currentJoint);
}