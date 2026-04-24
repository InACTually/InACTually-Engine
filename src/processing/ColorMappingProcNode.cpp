
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
#include "ColorMappingProcNode.hpp"


act::proc::ColorMappingProcNode::ColorMappingProcNode() : ProcNodeBase("ColorMapping") {
	m_drawSize = ivec2(200, 200);

	init = true;
	 

	auto hue = createNumberInput("hue", [&](float num) { this->onHue(num); });
	auto saturation = createNumberInput("saturation", [&](float num) { this->onSaturation(num); });
	auto lightness = createNumberInput("lightness", [&](float num) { this->onLightness(num); });

	m_colorPort = createColorOutput("color");


	m_hue = 0.0;
	m_saturation = 0.0;
	m_lightness = 0.0;
}

act::proc::ColorMappingProcNode::~ColorMappingProcNode() {
}

void act::proc::ColorMappingProcNode::update() {
}

void act::proc::ColorMappingProcNode::draw() {
	beginNodeDraw();
	 
		if (ImGui::ColorPicker3("Color", m_color.ptr())) {
			glm:vec3 c = m_color.get(ColorModel::CM_HSV);
			m_hue = c.x;
			m_saturation = c.y;
			m_lightness = c.z;
			sendColor();
			preventDrag(true);
		}
		else {
			preventDrag(false);
		}

	endNodeDraw();
}

ci::Json act::proc::ColorMappingProcNode::toParams() {
	ci::Json json = ci::Json::object();

	json["hue"] = m_hue;
	json["saturation"] = m_saturation;
	json["lightness"] = m_lightness;
	 
	return json;
}

void act::proc::ColorMappingProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "hue", m_hue);
	util::setValueFromJson(json, "saturation", m_saturation);
	util::setValueFromJson(json, "lightness", m_lightness);

	m_colorPort->send(m_color);
}


void act::proc::ColorMappingProcNode::onHue(float number) {
	m_hue = clamp(number, 0.0f, 1.0f);
	sendColor();
}

void act::proc::ColorMappingProcNode::onSaturation(float number) {
	m_saturation = clamp(number, 0.0f, 1.0f);
	sendColor();
}

void act::proc::ColorMappingProcNode::onLightness(float number) {
	m_lightness = clamp(number, 0.0f, 1.0f);
	sendColor();
}


void act::proc::ColorMappingProcNode::sendColor() {
	m_color = Color(ColorModel::CM_HSV, m_hue, m_saturation, m_lightness);
	m_colorPort->send(m_color);
}