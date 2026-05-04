
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "MovementDetectionProcNode.hpp"

#include "implot.h"


act::proc::MovementDetectionProcNode::MovementDetectionProcNode() : ProcNodeBase("MovementDetection") {

	m_displayScale = 0.8f;
	m_resizeScale = 0.3f;
	m_visualize = false;
	m_movementValue = 0.0f;
	m_useMOG2 = true;
	m_useKNN = false;
	m_movementThreshold = 0.0;

	auto image = createImageInput("image", [&](cv::UMat mat) { this->onMat(mat); });

	m_movementPort = createNumberOutput("movement value");
	m_imagePort = createImageOutput("visualized difference");
	m_isMovingPort = createBoolOutput("movement over threshold");

	init();
}

act::proc::MovementDetectionProcNode::~MovementDetectionProcNode() {
}

void act::proc::MovementDetectionProcNode::init() {
	if (m_useMOG2)
		m_bgSub = cv::createBackgroundSubtractorMOG2();
	else
		m_bgSub = cv::createBackgroundSubtractorKNN();
}

void act::proc::MovementDetectionProcNode::update() {
}

void act::proc::MovementDetectionProcNode::draw() {
	beginNodeDraw();

	ImGui::Checkbox("visualize", &m_visualize);

	ImGui::Value("movement: ", m_movementValue, "%.3f");

	if (m_texture && m_visualize) {
		gl::pushMatrices();
		gl::rotate(toRadians(180.0f));

		ci::vec2 texSize = Rectf(m_texture->getBounds()).getCenteredFit(ci::Rectf(ivec2(0, 0), m_drawSize), true).getSize();
		ImGui::Image(m_texture, texSize, vec2(1, 1), vec2(0, 0));


		gl::pushMatrices();
	}

	ImGui::SetNextItemWidth(500);
	preventDrag(ImGui::SliderFloat("resize", &m_resizeScale, 0.1f, 1.2f));

	if (ImGui::Checkbox("use MOG2", &m_useMOG2)) {
		m_useKNN = !m_useMOG2;
		init();
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("use KNN", &m_useKNN)) {
		m_useMOG2 = !m_useKNN;
		init();
	}
	ImGui::SetNextItemWidth(500);
	preventDrag(ImGui::SliderFloat("movement threshold", &m_movementThreshold, 0.00f, 0.50f));

	endNodeDraw();
}

void act::proc::MovementDetectionProcNode::onMat(cv::UMat event) {
	cv::UMat gray;

	int resizedCols = event.cols * m_resizeScale;
	if (resizedCols < 1)
		resizedCols = 1;
	int resizedRows = event.rows * m_resizeScale;
	if(resizedRows < 1)
		resizedRows = 1;
	cv::resize(event, gray, cv::Size(resizedCols, resizedRows));
	float calcScale = 1.0f / m_resizeScale;
	if(gray.type() != CV_8UC1)
	try {
		cv::cvtColor(gray, gray, cv::COLOR_BGR2GRAY);
	}
	catch (cv::Exception exc) {
		CI_LOG_E(exc.what());
	}
	cv::UMat mask;
	m_bgSub->apply(gray, mask, 0);

	if (m_previous.size != gray.size) {
		gray.copyTo(m_previous);
	}

	
	m_movementValue = (float)cv::countNonZero(mask) / (mask.rows*mask.cols);

	m_movementPort->send(m_movementValue);


	if (m_movementValue >= m_movementThreshold) 
		m_isMovingPort->send(true);
	else
		m_isMovingPort->send(false);

	if (m_visualize) {
		m_imagePort->send(mask);

		m_texture = gl::Texture2d::create(fromOcv(mask));
	}
	
}

ci::Json act::proc::MovementDetectionProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["resizeScale"]			= m_resizeScale;
	json["visualize"]			= m_visualize;
	json["useMOG2"]				= m_useMOG2;
	json["movementThreshold"]	= m_movementThreshold;
	
	return json;
}

void act::proc::MovementDetectionProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "resizeScale", m_resizeScale);
	util::setValueFromJson(json, "visualize", m_visualize);
	util::setValueFromJson(json, "useMOG2", m_useMOG2);
	util::setValueFromJson(json, "movementThreshold", m_movementThreshold);
	m_useKNN = !m_useMOG2;
	init();
}
