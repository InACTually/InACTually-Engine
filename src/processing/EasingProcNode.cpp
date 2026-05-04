
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "EasingProcNode.hpp"
#include <ranges>

act::proc::EasingProcNode::EasingProcNode() : ProcNodeBase("Easing") {
	m_drawSize = ci::ivec2(200, 200);

	createEasingsList();

	m_value = 0.0f;
	//m_tween = ci::Tween<float>();


	m_valuePort = createNumberOutput("out");

	auto valInput = createNumberInput("in", [&](float value) {
		m_value = value;
		m_valuePort->send(ease(m_value));
	});
}

act::proc::EasingProcNode::~EasingProcNode() {
}

void act::proc::EasingProcNode::update() {
	//m_dimPort->send(m_dim);
}

void act::proc::EasingProcNode::draw() {
	beginNodeDraw();

	if (ImGui::Combo("Easing", &m_selectedEasing, m_easingNames)) {
		m_valuePort->send(ease(m_value));
	}

	endNodeDraw();
}

ci::Json act::proc::EasingProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["value"] = m_value;
	json["selectedEasing"] = m_selectedEasing;
	return json;
}

void act::proc::EasingProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "value", m_value);
	util::setValueFromJson(json, "selectedEasing", m_selectedEasing);
}

float act::proc::EasingProcNode::ease(float value)
{
	return m_easingsMap[m_easingNames[m_selectedEasing]](value);
}

void act::proc::EasingProcNode::setEasing(int index)
{
	if (index < 0 || index >= m_easingNames.size())
		return;

	m_selectedEasing = index;
	m_valuePort->send(ease(m_value));
}

void act::proc::EasingProcNode::createEasingsList()
{
	m_easingsMap["Back In"]		= [&](float v) { return ci::easeInBack(v); };
	m_easingsMap["Back Out"]	= [&](float v) { return ci::easeOutBack(v); };
	m_easingsMap["Back InOut"]	= [&](float v) { return ci::easeInOutBack(v); };

	m_easingsMap["Bounce In"]	= [&](float v) { return ci::easeInBounce(v); };
	m_easingsMap["Bounce Out"]	= [&](float v) { return ci::easeOutBounce(v); };
	m_easingsMap["Bounce InOut"]= [&](float v) { return ci::easeInOutBounce(v); };

	m_easingsMap["Circ In"]		= [&](float v) { return ci::easeInCirc(v); };
	m_easingsMap["Circ Out"]	= [&](float v) { return ci::easeOutCirc(v); };
	m_easingsMap["Circ InOut"]	= [&](float v) { return ci::easeInOutCirc(v); };

	m_easingsMap["Cubic In"]	= [&](float v) { return ci::easeInCubic(v); };
	m_easingsMap["Cubic Out"]	= [&](float v) { return ci::easeOutCubic(v); };
	m_easingsMap["Cubic InOut"]	= [&](float v) { return ci::easeInOutCubic(v); };

	m_easingsMap["Expo In"]		= [&](float v) { return ci::easeInExpo(v); };
	m_easingsMap["Expo Out"]	= [&](float v) { return ci::easeOutExpo(v); };
	m_easingsMap["Expo InOut"]	= [&](float v) { return ci::easeInOutExpo(v); };

	m_easingsMap["Quart In"]	= [&](float v) { return ci::easeInQuart(v); };
	m_easingsMap["Quart Out"]	= [&](float v) { return ci::easeOutQuart(v); };
	m_easingsMap["Quart InOut"]	= [&](float v) { return ci::easeInOutQuart(v); };

	m_easingsMap["Quint In"]	= [&](float v) { return ci::easeInQuint(v); };
	m_easingsMap["Quint Out"]	= [&](float v) { return ci::easeOutQuint(v); };
	m_easingsMap["Quint InOut"]	= [&](float v) { return ci::easeInOutQuint(v); };

	m_easingsMap["Sine In"]		= [&](float v) { return ci::easeInSine(v); };
	m_easingsMap["Sine Out"]	= [&](float v) { return ci::easeOutSine(v); };
	m_easingsMap["Sine InOut"]	= [&](float v) { return ci::easeInOutSine(v); };

	auto kv = std::views::keys(m_easingsMap);
	m_easingNames = { kv.begin(), kv.end() };
}
