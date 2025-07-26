
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021�2025 Lars Engeln, Fabian T�pfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2023

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "NoiseProcNode.hpp"

#include "cinder/Rand.h"
#include <float.h> 
#include <math.h>

act::proc::NoiseProcNode::NoiseProcNode() : ProcNodeBase("Noise") {
	m_drawSize = ivec2(100, 200);
	m_perlin = Perlin();
	m_random = Rand::randFloat(0.0f, 255.0f);

	m_show = false;

	auto min = InputPort<float>::create(PT_NUMBER, "min", [&](float min) { this->onMin(min); });
	m_inputPorts.push_back(min);
	auto max = InputPort<float>::create(PT_NUMBER, "max", [&](float max) { this->onMax(max); });
	m_inputPorts.push_back(max);

	auto speed = InputPort<float>::create(PT_NUMBER, "speed", [&](float speed) { this->onSpeed(speed); });
	m_inputPorts.push_back(speed);

	m_noiseOutPort = OutputPort<number>::create(PT_NUMBER, "noise");
	m_outputPorts.push_back(m_noiseOutPort);
}

act::proc::NoiseProcNode::~NoiseProcNode() {}


void act::proc::NoiseProcNode::update() {
	float low = 0.0f;
	float high = 1.0f;

	float amplitude = m_max - m_min;
	float perlin = std::clamp(m_perlin.fBm(ci::app::getElapsedSeconds() * 2.5f * m_speed + m_random) + 0.5f, low, high);
	m_noise = perlin * amplitude + m_min;

	m_noiseOutPort->send(m_noise);
}

void act::proc::NoiseProcNode::draw() {
	beginNodeDraw();

	ImGui::PushItemWidth(m_drawSize.x);
	ImGui::DragFloat("Noise", &m_noise);

	endNodeDraw();
}

void act::proc::NoiseProcNode::onMin(float min) {
	m_min = min;
}

void act::proc::NoiseProcNode::onMax(float max) {
	m_max = max;
}

void act::proc::NoiseProcNode::onSpeed(float speed) {
	m_speed = speed;
}

ci::Json act::proc::NoiseProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["dim"] = 0; //TODO
	return json;
}

void act::proc::NoiseProcNode::fromParams(ci::Json json) {}
