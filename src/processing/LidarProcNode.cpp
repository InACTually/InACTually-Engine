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
#include "LiDARProcNode.hpp"


act::proc::LidarProcNode::LidarProcNode() : ProcNodeBase("Lidar", NT_INPUT) {
	m_show = false;

	m_movementOutputPort = createNumberOutput("movement");
	m_blobAmountOutputPort = createNumberOutput("blob amount");
	m_imageOutputPort = createImageOutput("image");
}

act::proc::LidarProcNode::~LidarProcNode() {
}

void act::proc::LidarProcNode::setup(act::room::RoomManagers roomMgrs) {
	m_lidarMgr = roomMgrs.lidarMgr;

}

void act::proc::LidarProcNode::update() {
	m_movementOutputPort->send(m_lidarMgr->getMovement());
	m_blobAmountOutputPort->send(m_lidarMgr->getNumOfBlobs());
	m_imageOutputPort->send(m_lidarMgr->getImage());
}

void act::proc::LidarProcNode::draw() {
	beginNodeDraw();
	
	if (ImGui::Checkbox("show", &m_show)) {
		// checkbox was clicked
	}


	endNodeDraw();
}

ci::Json act::proc::LidarProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["show"] = m_show;
	return json;
}

void act::proc::LidarProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "show", m_show);
}
