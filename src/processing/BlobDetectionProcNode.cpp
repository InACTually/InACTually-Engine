
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022-2023

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "BlobDetectionProcNode.hpp"


act::proc::BlobDetectionProcNode::BlobDetectionProcNode() : ProcNodeBase("BlobDetection") {
	m_drawSize = glm::ivec2(400, 300);

	valuesChanged = false;

	m_detectShadows = false;
	m_threshold = 16.0f;
	m_historyLength = 500;
	m_learningRate = 0.25;

	m_bgModel = cv::createBackgroundSubtractorMOG2(m_historyLength, m_threshold, m_detectShadows);

	auto image = createImageInput("image", [&](cv::UMat mat) { this->onMat(mat); });

	m_fgMaskPort = createImageOutput("foreground mask");
	m_fgCutoutPort = createImageOutput("foreground cutout");
	m_bgCutoutPort = createImageOutput("background cutout");
}

act::proc::BlobDetectionProcNode::~BlobDetectionProcNode() {
}

void act::proc::BlobDetectionProcNode::update() {
}

void act::proc::BlobDetectionProcNode::draw() {
	beginNodeDraw();

	if (m_texture_fgMask) {
		ci::gl::pushMatrices();
		ci::gl::rotate(ci::toRadians(180.0f));

		ImGui::Image(m_texture_fgMask, glm::vec2(m_drawSize.x, m_drawSize.y), glm::vec2(1, 1), glm::vec2(0, 0));

		ci::gl::pushMatrices();
	}

	ImGui::PushItemWidth(300);

	bool prvntDrag = false;

	if (ImGui::SliderFloat("learning rate", &m_learningRate, 0, 1)){
		valuesChanged = true;
		prvntDrag = true;
	}

	preventDrag(prvntDrag);

	if (ImGui::InputInt("history length", &m_historyLength)) {
		if (m_historyLength < 0)
			m_historyLength *= (-1);
		valuesChanged = true;
	}

	if (ImGui::InputFloat("threshold", &m_threshold)) {
		if (m_threshold < 0.0f)
			m_threshold *= (-1.0f);
		valuesChanged = true;
	}

	if (ImGui::Checkbox("detect shadows (slower)", &m_detectShadows)) {
		valuesChanged = true;
	}

	if (valuesChanged){
		m_bgModel = cv::createBackgroundSubtractorKNN(m_historyLength, m_threshold, m_detectShadows);
		valuesChanged = false;
	}

	endNodeDraw();
}

void act::proc::BlobDetectionProcNode::onMat(cv::UMat event) {
	bool expected = false;
	if (!m_isProcessing.compare_exchange_strong(expected, true))
		return;
	m_isProcessing = false;

	if (event.empty() || event.cols < 100 || event.rows < 100 || event.cols > 4096 || event.rows > 4096)
		return;
	if (m_foregroundMask.empty()) {
		m_foregroundMask.create(event.size(), event.type());
	}
	else if (m_foregroundMask.size() != event.size() || m_foregroundMask.type() != event.type())
		return;

	try {
		GaussianBlur(event, m_foregroundMask, cv::Size(11, 11), 3.5, 3.5);
		m_bgModel->apply(m_foregroundMask, m_foregroundMask, (double)m_learningRate);
	
		threshold(m_foregroundMask, m_foregroundMask, 10, 255, cv::THRESH_BINARY);
		m_foregroundCutout = cv::UMat(event.size(), event.type(), cv::Scalar(0));

		event.copyTo(m_foregroundCutout, m_foregroundMask);

		bitwise_not(m_foregroundMask, m_backgroundMask);
		event.copyTo(m_backgroundCutout, m_backgroundMask);
	}
	catch (cv::Exception exc) {
		CI_LOG_E(exc.what());
	}

	//m_bgModel->getBackgroundImage(backgroundImg);
	//m_texture_bg = ci::gl::Texture2d::create(ci::fromOcv(backgroundImg));

	m_bgCutoutPort->send(m_backgroundCutout);
	m_fgCutoutPort->send(m_foregroundCutout);
	m_fgMaskPort->send(m_foregroundMask);

	ci::app::App::get()->dispatchAsync([this]() {
		if (!m_foregroundMask.empty())
			m_texture_fgMask = ci::gl::Texture2d::create(ci::fromOcv(m_foregroundMask));
		if (!m_backgroundCutout.empty())
			m_texture_bgCutout = ci::gl::Texture2d::create(ci::fromOcv(m_backgroundCutout));
		if (!m_foregroundCutout.empty())
			m_texture_fgCutout = ci::gl::Texture2d::create(ci::fromOcv(m_foregroundCutout)); 
		float sizeFactor = 0.4;
		if (m_texture_fgMask)
			m_drawSize = glm::ivec2(m_texture_fgMask->getWidth() * sizeFactor, m_texture_fgMask->getHeight() * sizeFactor);
	});
}

ci::Json act::proc::BlobDetectionProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["historyLength"]	= m_historyLength;
	json["threshold"]		= m_threshold;
	json["detectShadows"]	= m_detectShadows;
	return json;
}

void act::proc::BlobDetectionProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "historyLength",	m_historyLength);
	util::setValueFromJson(json, "threshold",		m_threshold);
	util::setValueFromJson(json, "detectShadows",	m_detectShadows);
}
