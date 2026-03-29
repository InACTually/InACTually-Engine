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
	m_drawSize = ci::ivec2(200, 200);

	createNumberInput("value",			[&](number val) { m_mixValue = val; });

	createImageInput("primary image",	[&](image img)	{ m_primaryImage = img; mix(); });
	createImageInput("secondary image",	[&](image img)	{ m_secondaryImage = img; });

	m_output = createImageOutput("mixed image");
}

act::proc::VideoMixerProcNode::~VideoMixerProcNode() {
}

void act::proc::VideoMixerProcNode::update() {
	
}

void act::proc::VideoMixerProcNode::draw() {
	beginNodeDraw();

	if (ImGui::SliderFloat("mix value", &m_mixValue, 0.0f, 1.0f))
		mix();

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
	if (m_primaryImage.empty() && m_secondaryImage.empty())
		return;
	if (m_primaryImage.empty()) {
		m_output->send(m_secondaryImage);
		return;
	}
	if (m_secondaryImage.empty()) {
		m_output->send(m_primaryImage);
		return;
	}
	if(m_mixedImage.empty())
		m_mixedImage = cv::UMat(m_primaryImage.size(), m_primaryImage.type());

	float alpha = std::clamp(m_mixValue, 0.0f, 1.0f);
	cv::addWeighted(m_primaryImage, alpha, m_secondaryImage, 1.0f - alpha, 0.0f, m_mixedImage);
	m_output->send(m_mixedImage);
}
