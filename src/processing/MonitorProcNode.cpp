
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021-2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2022, 2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "MonitorProcNode.hpp"
#include "WindowData.hpp"

#include <opencv2/core/opengl.hpp>

act::proc::MonitorProcNode::MonitorProcNode() : ProcNodeBase("Monitor") {
	m_drawSize = glm::ivec2(400, 300);
	m_show = true;
	m_display = false;
	m_displayScale = 0.8f;
	m_fullscreen = false;

	m_textureHelper = TextureHelper::create();
	
	auto image = createImageInput("image", [&](cv::UMat mat) { this->onMat(mat); });

	m_imagePort = createImageOutput("pass-through image");
	m_texturePort = OutputPort<ci::gl::Texture2dRef>::create(PT_IMAGE, "texture");
}

act::proc::MonitorProcNode::~MonitorProcNode() {
}

void act::proc::MonitorProcNode::setup(act::room::RoomManagersRef roomMgrs) {
	m_displayMgr = roomMgrs->displayMgr;
}

void act::proc::MonitorProcNode::update() {
}

void act::proc::MonitorProcNode::draw() {
	beginNodeDraw();

	ImGui::Checkbox("show", &m_show);
	ImGui::SameLine();
	if (ImGui::Checkbox("to display", &m_display)) {
		if (m_display) {
			m_displayMgr->addSource(m_texturePort);
		}
		else {
			m_displayMgr->removeSource(m_texturePort);
		}
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("to projector", &m_projector)) {
		if (m_projector) {
			auto projector = m_projectorMgr->getProjectorByIndex(0);
			if (projector)
				projector->getImageInputPort()->connect(m_imagePort);
		}
		else {
			auto projector = m_projectorMgr->getProjectorByIndex(0);
			if (projector)
				projector->getImageInputPort()->disconnect(m_imagePort);
		}
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("fullscreen", &m_fullscreen)) {
		if (m_fullscreen) {
			ci::app::getWindow()->getUserData<act::WindowData>()->setIsFullscreen(true);
		}
		else {
			ci::app::getWindow()->getUserData<act::WindowData>()->setIsFullscreen(false);
			ci::app::getWindow()->getUserData<act::WindowData>()->setFullscreenTex(nullptr);
		}
	}
	
	if (m_show && !m_textureHelper->getTexture().empty()) {
		ci::gl::pushMatrices();
		//ci::gl::rotate(ci::toRadians(180.0f));
		
		ImGui::Image(m_textureHelper->getTexture().texId(), m_drawSize, glm::vec2(1, 1), glm::vec2(0, 0));
		ImGui::Indent(adaptSize(m_drawSize).x - m_drawSize.x);

		ci::gl::popMatrices();
	}

	endNodeDraw();
}

void act::proc::MonitorProcNode::onMat(cv::UMat event) {
	m_imagePort->send(event);


	if (m_show || m_display || m_fullscreen) {
		m_textureHelper->toTextureAsync(event, [this](cv::ogl::Texture2D texture) {

			m_drawSize = glm::ivec2(texture.cols(), texture.rows());

			auto windowData = ci::app::getWindow()->getUserData<act::WindowData>();

			if (m_fullscreen)
				windowData->setFullscreenTex(m_textureHelper->getTextureRef());
			
			if (m_display)
				m_texturePort->send(m_textureHelper->getTextureRef());
		});
	}
}

glm::ivec2 act::proc::MonitorProcNode::adaptSize(glm::ivec2 size) {
	return (size / 100 + 1) * 100;
}

ci::Json act::proc::MonitorProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["show"]	= m_show;
	json["display"] = m_display;
	json["fullscreen"] = m_fullscreen;
	return json;
}

void act::proc::MonitorProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "show", m_show);
	util::setValueFromJson(json, "display", m_display);
	util::setValueFromJson(json, "fullscreen", m_fullscreen);
	if(m_fullscreen)
		ci::app::getWindow()->getUserData<act::WindowData>()->setIsFullscreen(true);
}