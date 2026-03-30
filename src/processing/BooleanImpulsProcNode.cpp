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
#include "BooleanImpulsProcNode.hpp"
#include "cinder\Rand.h"

act::proc::BooleanImpulsProcNode::BooleanImpulsProcNode() : ProcNodeBase("BooleanImpuls") {
	m_drawSize = ci::ivec2(200, 200);
	
	m_output = createBoolOutput("impuls");
	createBoolInput("continuos", [&](bool val) { 
		if(val != m_value) {
			m_value = val;
			if (val)
				m_output->send(val);
		}
	});
}

act::proc::BooleanImpulsProcNode::~BooleanImpulsProcNode() {
}

void act::proc::BooleanImpulsProcNode::update() {
	
}

void act::proc::BooleanImpulsProcNode::draw() {
	beginNodeDraw();

	ImGui::Text(m_value ? "true" : "false");

	endNodeDraw();
}


ci::Json act::proc::BooleanImpulsProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["value"] = m_value;
	return json;
}

void act::proc::BooleanImpulsProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "value", m_value);
}
