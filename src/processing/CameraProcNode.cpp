
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021-2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021, 2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "CameraProcNode.hpp"


act::proc::CameraProcNode::CameraProcNode() : ProcNodeBase("Camera", NT_INPUT) {
	m_captureSize = glm::ivec2(1920, 1080);
	m_drawSize = glm::ivec2(m_captureSize.x * 0.25, m_captureSize.y * 0.25);

	m_show = false;
	m_selectedCamera = 0;

	try {
		m_textureHelper = proc::TextureHelper::create();
	}
	catch (const std::exception& e) {
		CI_LOG_E("Failed to create texture helper: " << e.what());
	}
	catch (...) {
		CI_LOG_E("Failed to create texture helper");
	}

	m_cameraImageInPort = createImageInput("cameraImage", [&](cv::UMat image) {
		if(image.empty())
			return;

		if (m_cameraRoomNode)
			m_cameraImageOutPort->send(image); // , m_cameraRoomNode);

		if (m_show && !image.empty()) {
			m_textureHelper->toTextureAsync(image, [this](cv::ogl::Texture2D texture) {});	
		}
	}, false);

	m_cameraImageOutPort = createImageOutput("cameraImage");

	m_cameraImageOutPort->setConnectionCB([]() {
		// someone connected to port
	});
}

act::proc::CameraProcNode::~CameraProcNode() {
	if (m_cameraRoomNode)
		m_cameraRoomNode->getCameraImagePort()->disconnect(m_cameraImageInPort);
}

void act::proc::CameraProcNode::setup(act::room::RoomManagersRef roomMgrs) {
	m_camMgr = roomMgrs->cameraMgr;

	auto camera = m_camMgr->getCameraByIndex(m_selectedCamera);
	if (camera) {
		attachCamera(camera);
	}
}

void act::proc::CameraProcNode::update() {
	if (m_cameraRoomNode) {
		m_cameraRoomNode->setIsHighlighted(m_isHovered);
		m_cameraRoomNode->setIsShowingDetails(m_isSelected);
	}
}

void act::proc::CameraProcNode::draw() {
	beginNodeDraw();
	
	if (ImGui::Checkbox("show", &m_show)) {
		// checkbox was clicked
	}

	if (m_camMgr->getUsedCameraNames().empty())
		ImGui::Text("No CameraDevice has been set up.");
	else {
		ImGui::SetNextItemWidth(m_drawSize.x - ImGui::CalcTextSize("CameraDevice").x);
		if (ImGui::Combo("CameraDevice", &m_selectedCamera, m_camMgr->getUsedCameraNames())) {
			attachCamera(m_camMgr->getCameraByIndex(m_selectedCamera));
		}
	}

	if (m_show && !m_textureHelper->getTexture().empty()) {
		ci::gl::pushMatrices();
		ci::gl::rotate(ci::toRadians(180.0f));
		ImGui::Image(m_textureHelper->getTexture().texId(), m_drawSize, glm::vec2(0, 0), glm::vec2(1, 1));
		ci::gl::pushMatrices();
	}

	endNodeDraw();
}



ci::Json act::proc::CameraProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["selectedDevice"] = m_selectedCameraName;
	json["show"] = m_show;
	return json;
}

void act::proc::CameraProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "show", m_show);

	if (util::setValueFromJson(json, "selectedDevice", m_selectedCameraName)) {
		auto&& port = m_camMgr->getCameraPort(m_selectedCameraName);
		if (port)
			port->connect(m_cameraImageInPort);

		m_selectedCamera = 0;
		for (auto&& name : m_camMgr->getUsedCameraNames()) {
			if (name == m_selectedCameraName)
				break;
			m_selectedCamera++;
		}
	}

	auto camera = m_camMgr->getCameraByIndex(m_selectedCamera);
	if (camera) {
		attachCamera(camera);
	}
}

void act::proc::CameraProcNode::attachCamera(act::room::CameraRoomNodeRef camera) {

	if (m_cameraRoomNode) {
			m_cameraRoomNode->getCameraImagePort()->disconnect(m_cameraImageInPort);
	}
	if (camera) {
		m_cameraRoomNode = camera;
		m_selectedCameraName = m_cameraRoomNode->getName();
		if (m_cameraRoomNode) {
			m_cameraRoomNode->getCameraImagePort()->connect(m_cameraImageInPort);
		}
	}
}
