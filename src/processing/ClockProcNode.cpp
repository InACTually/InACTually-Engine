
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2025-2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "ClockProcNode.hpp"
#include <ranges>

act::proc::ClockProcNode::ClockProcNode() : ProcNodeBase("Clock") {
	m_drawSize = ci::ivec2(200, 200);

	m_timeUntil = 10.f;
	m_value = 0.0f;

	m_startedAt = app::getElapsedSeconds();

	createBoolInput("fire", [&](bool val) { if (val) start(); });
	createBoolInput("reset", [&](bool val) { if (val) stop(); });

	m_valuePort		 = createNumberOutput("out");
	m_bangPort		 = createBoolOutput("bang");
}

act::proc::ClockProcNode::~ClockProcNode() {
}

void act::proc::ClockProcNode::update() {
	if (m_hasStarted) {
		m_elapsed = (app::getElapsedSeconds() - m_startedAt);
		m_value = m_elapsed / m_timeUntil;
	}
	

	if(m_value <= 1.f) {
		m_valuePort->send(m_value);
	}
	else
	{
		if (!m_bang) {
			m_bang = true;
			m_bangPort->send(true);
		}
		m_valuePort->send(m_value);
		stop();
	}
}

void act::proc::ClockProcNode::draw() {
	beginNodeDraw();

	if (!m_hasStarted) {
		if (ImGui::Button("start")) {
			start();
		}
	}
	else {
		if (ImGui::Button("stop")) {
			stop();
		}
	}

	ImGui::BeginDisabled();
	if (ImGui::SliderFloat("", &m_value, 0.0f, 1.0f)) {
	}
	ImGui::EndDisabled();

	if (ImGui::SliderFloat("seconds", &m_timeUntil, 0.0f, 3600.0f)) {
	}
	ImGui::PushID("seconds");
	ImGui::InputFloat("##xx", &m_timeUntil, 0.0f, 36000.0f);
	ImGui::PopID();

	endNodeDraw();
}

void act::proc::ClockProcNode::start()
{
	if (m_hasStarted)
		return; 

	m_hasStarted = true;
	m_value = 0.0f;
	m_startedAt = app::getElapsedSeconds();
}

void act::proc::ClockProcNode::stop()
{
	m_hasStarted = false;
	m_bang = false;
}

ci::Json act::proc::ClockProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["value"] = m_value;
	json["timeUntil"] = m_timeUntil;
	return json;
}

void act::proc::ClockProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "value", m_value);
	util::setValueFromJson(json, "timeUntil", m_timeUntil);
}
