
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021�2025 Lars Engeln, Fabian T�pfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021

	contributors:
	Fabian T�pfer
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "FaceEmotionProcNode.hpp"

act::proc::FaceEmotionProcNode::FaceEmotionProcNode() : ProcNodeBase("FaceEmotion") {

	m_displayScale = 0.8f;
	
	auto image = InputPort<cv::UMat>::create(PT_IMAGE, "image", [&](cv::UMat mat) { this->onMat(mat); });
	m_inputPorts.push_back(image);

	m_imagePort = OutputPort<cv::UMat>::create(PT_IMAGE, "pass-through image");
	m_outputPorts.push_back(m_imagePort);

	
	m_emotionPort = OutputPort<feature>::create(PT_FEATURE, "emotion");
	m_outputPorts.push_back(m_emotionPort);

	//init network
	std::string onnxFile = ci::app::getAssetPath("3rd/emotion/emotion-ferplus-8.onnx").string();
	m_network = cv::dnn::readNetFromONNX(onnxFile);

	if (m_network.empty()) {
		CI_LOG_E("Failed to load network");
	};
	
	m_emotions = { "Neutral", "Happy", "Surprise", "Sad", "Anger", "Disgust", "Fear", "Contempt" };
	m_currentEmotion = std::make_pair("Idle",0.0f);
}

act::proc::FaceEmotionProcNode::~FaceEmotionProcNode() {
}

void act::proc::FaceEmotionProcNode::update() {
}

void act::proc::FaceEmotionProcNode::draw() {
	beginNodeDraw();

	ImGui::Checkbox("show", &m_show);

	ImGui::TextUnformatted("Current Emotion");
	ImGui::TextUnformatted(m_currentEmotion.first.c_str());
	ImGui::Text("%f",m_currentEmotion.second);
	
	if (m_show && m_texture) {
		gl::pushMatrices();
		gl::rotate(toRadians(180.0f));

		ci::vec2 texSize = Rectf(m_texture->getBounds()).getCenteredFit(ci::Rectf(ivec2(0, 0), m_drawSize), true).getSize();

		ImGui::Image(m_texture, texSize, vec2(1, 1), vec2(0, 0));

		//ImGui::Indent(adaptSize(displaySize).x - displaySize.x);

		gl::pushMatrices();
	}
	
	endNodeDraw();
}

void act::proc::FaceEmotionProcNode::onMat(cv::UMat event) {
	m_imagePort->send(event);
	if (m_show) {
		m_texture = gl::Texture2d::create(fromOcv(event));
	}

	cv::Mat emotions = detectCurrentEmotions(event);
	m_currentEmotion = getCurrentEmotion(softmax(&emotions));

	m_emotionPort->send(m_currentEmotion);
}

ci::ivec2 act::proc::FaceEmotionProcNode::adaptSize(ci::ivec2 size) {
	return (size / 100 + 1) * 100;
}

cv::Mat	act::proc::FaceEmotionProcNode::detectCurrentEmotions(cv::UMat uframe) {

	//first detect faces
	//convert to 64 by 64 pixel grey-scale image
	//feed into onnx model
	cv::Mat frame;

	cv::cvtColor(uframe, uframe, cv::COLOR_RGB2GRAY);
	cv::equalizeHist(uframe, uframe);
	frame = uframe.getMat(cv::ACCESS_RW);

	frame = cv::dnn::blobFromImage(frame, 1.0f, cv::Size(64, 64));

	m_network.setInput(frame);

	cv::Mat out = m_network.forward();
	return out;
}

act::proc::feature act::proc::FaceEmotionProcNode::getCurrentEmotion(std::vector<float> result) {
	float min = 0.0f;
	float max = 0.0f;
	int minI = 0;
	int maxI = 0;

	for (int i = 0; i < result.size(); i++) {
		if (result[i] > max) {
			max = result[i];
			maxI = i;
		}
	}
	if (maxI < 0 || maxI >= m_emotions.size()) {
		CI_LOG_E("[FaceEmotionProcNode] error while calculating current emotion");
		maxI = 0;
	};

	return std::make_pair(m_emotions[maxI], max);
};

std::vector<float> act::proc::FaceEmotionProcNode::softmax(cv::Mat* mat) {

	int i;
	double m, sum, constant;

	std::vector<float> results;

	m = -INFINITY;
	for (i = 0; i < mat->cols; ++i) {
		if (m < mat->at<float>(0, i)){

			m = mat->at<float>(0, i);
		}
	}

	sum = 0.0;
	for (i = 0; i < mat->cols; ++i) {
		sum += exp(mat->at<float>(0, i) - m);
	}

	constant = m + std::log(sum);
	for (i = 0; i < mat->cols; ++i) {
		results.push_back( exp(mat->at<float>(0, i) - constant));
	}

	return results;
}