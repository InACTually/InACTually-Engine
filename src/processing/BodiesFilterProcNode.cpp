
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022-2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "BodiesFilterProcNode.hpp"


act::proc::BodiesFilterProcNode::BodiesFilterProcNode() : ProcNodeBase("BodiesFilter") {
	m_drawSize = glm::ivec2(250, 250);

	m_nearestToPosition = glm::vec3(0.0f);
	m_currentPosition = glm::vec3(0.0f);
	m_nearestToPositionDistance = 1000.0f;

	m_isBody = false;

	m_bodyOutPort = OutputPort<room::BodyRef>::create(PT_BODY, "filtered Body");
	m_outputPorts.push_back(m_bodyOutPort);

	auto bodies = InputPort<room::BodyRefList>::create(PT_BODYLIST, "Bodies", [&](room::BodyRefList bodies) { this->onBodies(bodies); });
	m_inputPorts.push_back(bodies);
}

act::proc::BodiesFilterProcNode::~BodiesFilterProcNode() {
}

void act::proc::BodiesFilterProcNode::setup() {
}

void act::proc::BodiesFilterProcNode::update() {
}

void act::proc::BodiesFilterProcNode::draw() {
	beginNodeDraw();

	ImGui::SetNextItemWidth(m_drawSize.x - ImGui::CalcTextSize("select body").x);

	ImGui::Combo("select body", &m_currentBody, m_bodySelection);

	if (m_bodySelection[m_currentBody] == "Position")
		drawPickPosition();
	
	ImGui::NewLine();

	ImGui::Text("incoming Bodies: %d", m_numberOfBodies);
	if (m_isBody) {
		ImGui::Text("outgoing Body: yes! ");
	}
	else {
		ImGui::Text("outgoing Body: no");
	}

	endNodeDraw();
}

void act::proc::BodiesFilterProcNode::drawPickPosition()
{
	ImGui::DragFloat3("near to", &m_nearestToPosition);
	ImGui::DragFloat("distance", &m_nearestToPositionDistance);

	for (int i = 0; i < m_bodies.size(); i++) {
		ImGui::PushID(i);
		auto joint = m_bodies[i]->joints[K4ABT_JOINT_SPINE_CHEST];
		glm::vec3 position = glm::vec3(joint->position.x, joint->position.y, joint->position.z);
		if (ImGui::Button("pick position"))
			m_nearestToPosition = position;
		ImGui::SameLine();
		std::stringstream strstr;
		strstr << i << ": " << std::setprecision(3) << position.x << ", " << position.y << ", " << position.z;
		ImGui::Text(strstr.str().c_str());
		ImGui::PopID();
	}
}

void act::proc::BodiesFilterProcNode::onBodies(room::BodyRefList event)
{
	m_bodies = event;
	m_numberOfBodies = event.size();
	if (m_numberOfBodies == 0)
		return;

	room::BodyRef body;
	m_isBody = false;

	switch (m_currentBody)
	{
	case 0:
		//Position
		
		body = findNearestBody(event);
		break;
	case 5:
		//Last
		{
			auto it = std::prev(event.end());

			body = *it;
			m_isBody = true;
		}
		break;
	default:
		//First - 0, Second - 1 and Third - 2
		int numBodies = m_currentBody;
		if (event.size() >= (numBodies))
		{
			auto it = event.begin();
			std::advance(it, m_currentBody - 1);
			body = *it;
			m_isBody = true;
		}
		break;
	}

	if (m_isBody)
		m_bodyOutPort->send(body);
}

act::room::BodyRef act::proc::BodiesFilterProcNode::findNearestBody(room::BodyRefList bodies)
{
	m_isBody = false;
	float distance = m_nearestToPositionDistance;
	room::BodyRef body;
	for (auto&& b : bodies) {
		if (!b)
			continue;

		m_currentPosition = b->joints[K4ABT_JOINT_SPINE_CHEST]->position;
		
		float d = ci::distance(glm::vec2(m_currentPosition.x, m_currentPosition.z), glm::vec2(m_nearestToPosition.x, m_nearestToPosition.z));
		if (d < distance) {
			distance = d;
			body = b;
			m_isBody = true;
		}
		else {
			m_currentPosition = glm::vec3(0.0f);
		}
	}
	return body;
}

ci::Json act::proc::BodiesFilterProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["selectedBody"] = m_currentBody;
	json["nearestToPosition"] = util::valueToJson(m_nearestToPosition);
	json["nearestToPositionThreshold"] = m_nearestToPositionDistance;
	return json;
}

void act::proc::BodiesFilterProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "selectedBody", m_currentBody);
	util::setValueFromJson(json, "nearestToPosition", m_nearestToPosition);
	util::setValueFromJson(json, "nearestToPositionThreshold", m_nearestToPositionDistance);
}