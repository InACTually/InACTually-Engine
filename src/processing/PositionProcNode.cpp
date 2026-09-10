
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021â€“2025 Lars Engeln, Fabian TÃ¶pfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "PositionProcNode.hpp"

act::proc::PositionProcNode::PositionProcNode() : ProcNodeBase("Position") {
	m_drawSize = glm::ivec2(200, 200);

	m_t = 0.0f;

	m_tPort = createNumberInput("t", [&](float t) {
		m_t = t; m_positionPort->send(m_positionRoomNode->evaluatePosition(m_t));
		});

	m_positionPort = createVec3Output("position");
}

act::proc::PositionProcNode::~PositionProcNode() {
}

void act::proc::PositionProcNode::setup(act::room::RoomManagersRef dMgrs)
{
	m_posMgr = dMgrs->positionMgr;
	auto node = std::dynamic_pointer_cast<room::PositionRoomNode>(m_posMgr->addPosition(glm::vec3(0.0f)));
	if (node)
		m_positionRoomNode = node;
}

void act::proc::PositionProcNode::update() {
	if (m_positionRoomNode) {
		m_positionRoomNode->setIsHighlighted(m_isHovered);
		m_positionRoomNode->setIsShowingDetails(m_isSelected);
	}
}

void act::proc::PositionProcNode::draw() {

	beginNodeDraw();

	if(ImGui::DragFloat3("Position", &m_pos, 0.01f)) {
		preventDrag(true);
		m_positionRoomNode->setPosition(m_pos);
		m_positionPort->send(m_positionRoomNode->evaluatePosition(m_t));
	}
	else {
		preventDrag(false);
	}

	m_tPort->draw(5, true);
	if (ImGui::DragFloat("at path", &m_t, 0.001f)) {
		preventDrag(true);
		m_positionPort->send(m_positionRoomNode->evaluatePosition(m_t));
	}
	else {
		preventDrag(false);
	}

	bool loop = m_positionRoomNode->getIsLooping();
	if (ImGui::Checkbox("loop", &loop)) {
		m_positionRoomNode->setIsLooping(loop);
		m_positionPort->send(m_positionRoomNode->evaluatePosition(m_t));
	}
	ImGui::SameLine();
	int degree = m_positionRoomNode->getDegree();
	if (ImGui::InputInt("degree", &degree)) {
		m_positionRoomNode->setDegree(degree);
		m_positionPort->send(m_positionRoomNode->evaluatePosition(m_t));
	}
	ImGui::Spacing();
	if (ImGui::Button(ICON_FA_PLUS " add point")) {
		m_positionRoomNode->addControlPoint(m_positionRoomNode->getLastControlPoint());
		m_positionPort->send(m_positionRoomNode->evaluatePosition(m_t));
	}
	
	auto points = m_positionRoomNode->getControlPoints();
	for (int i = 0; i < points.size(); i++) {
		ImGui::PushID(i);
		if (ImGui::Button(ICON_FA_MINUS)) {
			m_positionRoomNode->removeControlPoint(i);
		}
		ImGui::SameLine();
		glm::vec3 pt = points[i];
		if (ImGui::DragFloat3("", &pt, 0.01f)) {
			preventDrag(true);
			m_positionRoomNode->setControlPoint(i, pt);
			m_positionPort->send(m_positionRoomNode->evaluatePosition(m_t));
		}
		else {
			preventDrag(false);
		}
		ImGui::PopID();
	}
	
	endNodeDraw(false, true);
}

ci::Json act::proc::PositionProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["t"] = m_t;
	if(m_positionRoomNode)
		json["positionNodeUID"] = m_positionRoomNode->getUID();
	return json;
}

void act::proc::PositionProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "t", m_t);
	act::UID uid = "";
	util::setValueFromJson(json, "positionNodeUID", uid);

	m_posMgr->removeNode(m_positionRoomNode->getUID());
	m_positionRoomNode = std::dynamic_pointer_cast<room::PositionRoomNode>(m_posMgr->getNodeByUID(uid));
	if(!m_positionRoomNode)
		m_positionRoomNode = std::dynamic_pointer_cast<room::PositionRoomNode>(m_posMgr->addPosition(glm::vec3(0)));

	m_pos = m_positionRoomNode->getPosition();

	m_positionPort->send(m_positionRoomNode->evaluatePosition(m_t));
}
