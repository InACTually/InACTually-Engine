
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021�2025 Lars Engeln, Fabian T�pfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2022

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "MonitorProcNode.hpp"

act::proc::MonitorProcNode::MonitorProcNode() : ProcNodeBase("Monitor") {
	m_drawSize = ivec2(400, 300);
	m_show = true;
	m_display = false;
	m_displayScale = 0.8f;
	
	auto image = InputPort<cv::UMat>::create(PT_IMAGE, "image", [&](cv::UMat mat) { this->onMat(mat); });
	m_inputPorts.push_back(image);

	m_imagePort = OutputPort<cv::UMat>::create(PT_IMAGE, "pass-through image");
	m_texturePort = OutputPort<ci::gl::Texture2dRef>::create(PT_IMAGE, "texture");
	m_outputPorts.push_back(m_imagePort);
}

act::proc::MonitorProcNode::~MonitorProcNode() {
}

void act::proc::MonitorProcNode::setup(act::room::RoomManagers roomMgrs) {
	m_displayMgr = roomMgrs.displayMgr;
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
	
	if (m_show && m_texture) {
		gl::pushMatrices();
		//gl::rotate(toRadians(180.0f));
		
		ImGui::Image(m_texture, m_drawSize, vec2(1, 1), vec2(0, 0));
		ImGui::Indent(adaptSize(m_drawSize).x - m_drawSize.x);

		gl::pushMatrices();
	}

	endNodeDraw();
}

void act::proc::MonitorProcNode::onMat(cv::UMat event) {
	m_imagePort->send(event);
	if (m_show || m_display) {
		m_texture = gl::Texture2d::create(fromOcv(event));
		m_drawSize = ivec2(m_texture->getWidth(), m_texture->getHeight());
	}
	if (m_display)
		m_texturePort->send(m_texture);
}

ci::ivec2 act::proc::MonitorProcNode::adaptSize(ci::ivec2 size) {
	return (size / 100 + 1) * 100;
}

ci::Json act::proc::MonitorProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["show"]	= m_show;
	json["display"] = m_display;
	return json;
}

void act::proc::MonitorProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "show", m_show);
	util::setValueFromJson(json, "display", m_display);
}