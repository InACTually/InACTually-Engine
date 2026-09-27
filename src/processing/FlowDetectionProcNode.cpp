
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021, 2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "FlowDetectionProcNode.hpp"

#include "implot.h"

act::proc::FlowDetectionProcNode::FlowDetectionProcNode() : ProcNodeBase("FlowDetection") {

	m_displayScale = 0.8f;
	m_resizeScale = 0.3f;
	m_visualize = false;
	m_movementValue = 0.0f;

	m_textureHelper = TextureHelper::create();

	auto image = createImageInput("image", [&](cv::UMat mat) { this->onMat(mat); });

	m_movementPort = createNumberOutput("movement value");
	m_flowPort = createImageOutput("flow image");
	m_imagePort = createImageOutput("visualized flow");
}

act::proc::FlowDetectionProcNode::~FlowDetectionProcNode() {
}

void act::proc::FlowDetectionProcNode::update() {
}

void act::proc::FlowDetectionProcNode::draw() {
	beginNodeDraw();

	ImGui::Checkbox("visualize", &m_visualize);

	ImGui::Value("movement: ", m_movementValue, "%.3f");

	if (m_textureHelper->hasTexture() && m_visualize) {
		ci::gl::pushMatrices();
		ci::gl::rotate(ci::toRadians(180.0f));

		glm::vec2 texSize = ci::Rectf(glm::vec2(0, 0), ci::fromOcv(m_textureHelper->getTexture().size())).getCenteredFit(ci::Rectf(glm::ivec2(0, 0), m_drawSize), true).getSize();
		ImGui::Image(m_textureHelper->getTexture().texId(), texSize);

		ci::gl::pushMatrices();
	}

	ImGui::SetNextItemWidth(500);
	preventDrag(ImGui::SliderFloat("resize", &m_resizeScale, 0.1f, 1.2f));

	endNodeDraw();
}

void act::proc::FlowDetectionProcNode::onMat(cv::UMat event) {
	cv::UMat gray;

	int resizedCols = event.cols * m_resizeScale;
	if (resizedCols < 1)
		resizedCols = 1;
	int resizedRows = event.rows * m_resizeScale;
	if (resizedRows < 1)
		resizedRows = 1;
	cv::resize(event, gray, cv::Size(resizedCols, resizedRows));
	float calcScale = 1.0f / m_resizeScale;

	if (gray.type() != CV_8UC1)
		try {
		cv::cvtColor(gray, gray, cv::COLOR_BGR2GRAY);
	}
	catch (cv::Exception exc) {
		CI_LOG_F(exc.what());
	}

	if (m_previous.size != gray.size) {
		gray.copyTo(m_previous);
	}

	cv::UMat flow(m_previous.size(), CV_32FC2);
	calcOpticalFlowFarneback(m_previous, gray, flow, 0.5, 3, 15, 3, 5, 1.2, 0);
	m_previous = gray;

	cv::Mat flow_parts[2];
	cv::split(flow.getMat(cv::ACCESS_FAST), flow_parts);
	cv::Mat magnitude, angle;
	cv::cartToPolar(flow_parts[0], flow_parts[1], magnitude, angle, true);
	cv::normalize(magnitude, magnitude, 0.0f, 1.0f, cv::NORM_MINMAX);

	m_flowPort->send(magnitude.getUMat(cv::ACCESS_FAST));

	m_movementValue = cv::mean(magnitude)[0];
	m_movementPort->send(m_movementValue);

	if (m_visualize) {
		// visualization
		angle *= ((1.f / 360.f) * (180.f / 255.f));

		//build hsv image
		cv::Mat _hsv[3], hsv, hsv8;
		cv::Mat bgr;
		_hsv[0] = angle;
		_hsv[1] = cv::Mat::ones(angle.size(), CV_32F);
		_hsv[2] = magnitude;
		cv::merge(_hsv, 3, hsv);
		hsv.convertTo(hsv8, CV_8U, 255.0);
		cvtColor(hsv8, bgr, cv::COLOR_HSV2BGR);

		cv::UMat img = bgr.getUMat(cv::ACCESS_FAST);

		m_imagePort->send(img);
		if (m_visualize) {
			m_textureHelper->toTextureAsync(img);
		}
	}
}

ci::Json act::proc::FlowDetectionProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["resizeScale"]	= m_resizeScale;
	json["visualize"]	= m_visualize;
	return json;
}

void act::proc::FlowDetectionProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "resizeScale", m_resizeScale);
	util::setValueFromJson(json, "visualize", m_visualize);
}
