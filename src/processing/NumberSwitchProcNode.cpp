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
#include "NumberSwitchProcNode.hpp"
#include "cinder\Rand.h"

act::proc::NumberSwitchProcNode::NumberSwitchProcNode() : ProcNodeBase("NumberSwitch") {
	m_drawSize = ci::ivec2(200, 200);

	createNumberInput("in", [&](number val) { switchNumber(val); });

	m_outputs.push_back(createBoolOutput("1"));
	m_outputs.push_back(createBoolOutput("2"));
	m_outputs.push_back(createBoolOutput("3"));
}

act::proc::NumberSwitchProcNode::~NumberSwitchProcNode() {
}

void act::proc::NumberSwitchProcNode::update() {
	
}

void act::proc::NumberSwitchProcNode::draw() {
	beginNodeDraw();

	ImGui::Text(m_text.c_str());

	endNodeDraw();
}


ci::Json act::proc::NumberSwitchProcNode::toParams() {
	ci::Json json = ci::Json::object();
	return json;
}

void act::proc::NumberSwitchProcNode::fromParams(ci::Json json) {
}

void act::proc::NumberSwitchProcNode::switchNumber(number val)
{
	if (m_outputs.empty())
		return;

	if(val < 0.33f) {
		m_outputs[0]->send(true);
		m_outputs[1]->send(false);
		m_outputs[2]->send(false);
		m_text = "1";
	}
	else if(val < 0.66f) {
		m_outputs[0]->send(false);
		m_outputs[1]->send(true);
		m_outputs[2]->send(false);
		m_text = "2";
	}
	else {
		m_outputs[0]->send(false);
		m_outputs[1]->send(false);
		m_outputs[2]->send(true);
		m_text = "3";
	}
}
