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
#include "VideoMixerProcNode.hpp"
#include "cinder\Rand.h"

act::proc::VideoMixerProcNode::VideoMixerProcNode() : ProcNodeBase("VideoMixer") {
	m_drawSize = glm::ivec2(200, 200);

	createNumberInput("value",			[&](number val) {
		{
			std::lock_guard<std::mutex> lock(m_mixMutex);
			m_mixValue = val;
		}
		mix();
	});

	createImageInput("primary image",	[&](image img)	{
		{
			std::lock_guard<std::mutex> lock(m_mixMutex);
			m_primaryImage = img.clone();
		}
		mix();
	});
	createImageInput("secondary image",	[&](image img)	{
		std::lock_guard<std::mutex> lock(m_mixMutex);
		m_secondaryImage = img.clone();
	});

	m_output = createImageOutput("mixed image");
}

act::proc::VideoMixerProcNode::~VideoMixerProcNode() {
}

void act::proc::VideoMixerProcNode::update() {
	
}

void act::proc::VideoMixerProcNode::draw() {
	beginNodeDraw();

	float mixValue = 0.0f;
	{
		std::lock_guard<std::mutex> lock(m_mixMutex);
		mixValue = m_mixValue;
	}

	if (ImGui::SliderFloat("mix value", &mixValue, 0.0f, 1.0f)) {
		{
			std::lock_guard<std::mutex> lock(m_mixMutex);
			m_mixValue = mixValue;
		}
		mix();
	}

	endNodeDraw();
}


ci::Json act::proc::VideoMixerProcNode::toParams() {
	ci::Json json = ci::Json::object();
	return json;
}

void act::proc::VideoMixerProcNode::fromParams(ci::Json json) {
}

void act::proc::VideoMixerProcNode::mix()
{
	std::lock_guard<std::mutex> processLock(m_mixProcessMutex);

	image primaryImage;
	image secondaryImage;
	float mixValue = 0.0f;

	{
		std::lock_guard<std::mutex> lock(m_mixMutex);
		primaryImage = m_primaryImage;
		secondaryImage = m_secondaryImage;
		mixValue = m_mixValue;
	}

	if (primaryImage.empty() && secondaryImage.empty())
		return;
	if (primaryImage.empty()) {
		m_output->send(secondaryImage);
		return;
	}
	if (secondaryImage.empty()) {
		m_output->send(primaryImage);
		return;
	}

	if (primaryImage.size() != secondaryImage.size()) {
		image resizedSecondary;
		cv::resize(secondaryImage, resizedSecondary, primaryImage.size());
		secondaryImage = resizedSecondary;
	}

	if (primaryImage.channels() != secondaryImage.channels()) {
		image convertedSecondary;
		if (primaryImage.channels() == 4 && secondaryImage.channels() == 3)
			cv::cvtColor(secondaryImage, convertedSecondary, cv::COLOR_BGR2BGRA);
		else if (primaryImage.channels() == 3 && secondaryImage.channels() == 4)
			cv::cvtColor(secondaryImage, convertedSecondary, cv::COLOR_BGRA2BGR);
		else if (primaryImage.channels() == 1 && secondaryImage.channels() == 3)
			cv::cvtColor(secondaryImage, convertedSecondary, cv::COLOR_BGR2GRAY);
		else if (primaryImage.channels() == 1 && secondaryImage.channels() == 4)
			cv::cvtColor(secondaryImage, convertedSecondary, cv::COLOR_BGRA2GRAY);
		else if (primaryImage.channels() == 3 && secondaryImage.channels() == 1)
			cv::cvtColor(secondaryImage, convertedSecondary, cv::COLOR_GRAY2BGR);
		else if (primaryImage.channels() == 4 && secondaryImage.channels() == 1)
			cv::cvtColor(secondaryImage, convertedSecondary, cv::COLOR_GRAY2BGRA);
		else
			convertedSecondary = secondaryImage;
		secondaryImage = convertedSecondary;
	}

	if (primaryImage.depth() != secondaryImage.depth()) {
		image convertedDepth;
		secondaryImage.convertTo(convertedDepth, CV_MAKETYPE(primaryImage.depth(), secondaryImage.channels()));
		secondaryImage = convertedDepth;
	}

	image mixedImage;
	float alpha = std::clamp(mixValue, 0.0f, 1.0f);
	cv::addWeighted(primaryImage, alpha, secondaryImage, 1.0f - alpha, 0.0f, mixedImage);
	m_output->send(mixedImage);
}
