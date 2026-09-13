
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
#include "BoneVectorProcNode.hpp"


act::proc::BoneVectorProcNode::BoneVectorProcNode() : ProcNodeBase("BoneVector") {
	m_drawSize = glm::ivec2(250, 250);

	m_doNormalize = false;

	m_positionOutPort = createVec3Output("Position");

	auto skeleton = InputPort<room::BodyRef>::create(PT_BODY, "Body", [&](room::BodyRef body) { this->onSkeleton(body); });
	m_inputPorts.push_back(skeleton);

	m_fromJoint = K4ABT_JOINT_ELBOW_RIGHT;
	m_toJoint = K4ABT_JOINT_HAND_RIGHT;
}

act::proc::BoneVectorProcNode::~BoneVectorProcNode() {
}


void act::proc::BoneVectorProcNode::setup() {
}

void act::proc::BoneVectorProcNode::update() {
}

void act::proc::BoneVectorProcNode::draw() {
	beginNodeDraw();

	ImGui::SetNextItemWidth(m_drawSize.x - ImGui::CalcTextSize("from joint").x);
	ImGui::Combo("from joint", &m_fromJoint, m_jointSelection, IM_ARRAYSIZE(m_jointSelection));

	ImGui::SetNextItemWidth(m_drawSize.x - ImGui::CalcTextSize("from joint").x);
	ImGui::Combo("to joint", &m_toJoint, m_jointSelection, IM_ARRAYSIZE(m_jointSelection));

	ImGui::Checkbox("normalize", &m_doNormalize);

	if (m_fromJoint != m_toJoint) {
		ImGui::Text("current vector [xyz]: [ %3.2lf,  %3.2lf, %3.2lf ]", m_currentVec.x, m_currentVec.y, m_currentVec.z);
	}

	endNodeDraw();
}

void act::proc::BoneVectorProcNode::onSkeleton(room::BodyRef event) {
	if (!event || m_fromJoint == m_toJoint || event->joints.size() < m_fromJoint || event->joints.size() < m_toJoint)
		return;

	glm::vec3 from = event->joints[m_fromJoint]->position;
	glm::vec3 to = event->joints[m_toJoint]->position;

	m_currentVec = to - from;

	if (m_doNormalize)
		m_currentVec = ci::normalize(m_currentVec);

	m_positionOutPort->send(m_currentVec);
}


ci::Json act::proc::BoneVectorProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["fromJoint"] = m_fromJoint;
	json["toJoint"] = m_toJoint;
	json["doNormalize"] = m_doNormalize;
	return json;
}

void act::proc::BoneVectorProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "toJoint", m_toJoint);
	util::setValueFromJson(json, "fromJoint", m_fromJoint);
	util::setValueFromJson(json, "doNormalize", m_doNormalize);
}