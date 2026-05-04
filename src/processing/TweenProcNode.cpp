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
#include "TweenProcNode.hpp"

act::proc::TweenProcNode::TweenProcNode() : ProcNodeBase("Tween") {
	m_drawSize = ci::ivec2(200, 200);
	m_append = false;
	m_duration = 5;
	m_selectedEasingIndex = 0;
	m_easing = ci::easeNone;
	m_value = 0.0f;

	createEasings();
	
	createNumberInput("to value", [&](number val) {
		if (m_append) {
			ci::app::timeline().appendTo(&m_value, val, m_duration, m_easing).updateFn([&]() {
				m_atValuePort->send(m_value.value());
			});
		}
		else {
			ci::app::timeline().apply(&m_value, val, m_duration, m_easing).updateFn([&]() {
				m_atValuePort->send(m_value.value());
			});
		}
	});	

	m_atValuePort = createNumberOutput("at value");
}

act::proc::TweenProcNode::~TweenProcNode() {
}

void act::proc::TweenProcNode::setup(act::room::RoomManagers roomMgrs) {
}

void act::proc::TweenProcNode::update() {
}

void act::proc::TweenProcNode::draw() {
	beginNodeDraw();

	ImGui::Text(getName().c_str());

	bool prvntDrag = false;

	if (ImGui::Checkbox("append", &m_append)) {
		prvntDrag = true;
	}
	ImGui::SetNextItemWidth(m_drawSize.x);
	if (ImGui::DragFloat("duration", &m_duration)) {
		prvntDrag = true;
	}
	ImGui::SetNextItemWidth(m_drawSize.x);
	if (ImGui::Combo("Easing", &m_selectedEasingIndex, m_easingNames)) {
		m_easing = m_easingFunctions[m_selectedEasingIndex];
	}

	preventDrag(prvntDrag);

	endNodeDraw();
}

ci::Json act::proc::TweenProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["append"] = m_append;
	json["duration"] = m_duration;
	json["selectedEasingIndex"] = m_selectedEasingIndex;
	return json;
}

void act::proc::TweenProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "append", m_append);
	util::setValueFromJson(json, "duration", m_duration);
	util::setValueFromJson(json, "selectedEasingIndex", m_selectedEasingIndex);
	m_easing = m_easingFunctions[m_selectedEasingIndex];

}

void act::proc::TweenProcNode::createEasings()
{
	m_easingFunctions.push_back(ci::easeNone);
	m_easingNames.push_back("none");
	m_easingFunctions.push_back(ci::easeInOutSine);
	m_easingNames.push_back("sine");
	m_easingFunctions.push_back(ci::easeInOutQuad);
	m_easingNames.push_back("quad");
	m_easingFunctions.push_back(ci::easeInOutCubic);
	m_easingNames.push_back("cubic");
	m_easingFunctions.push_back(ci::easeInOutQuart);
	m_easingNames.push_back("quart");
	m_easingFunctions.push_back(ci::easeInOutQuint);
	m_easingNames.push_back("quint");
	m_easingFunctions.push_back(ci::easeInOutCirc);
	m_easingNames.push_back("circ");
}
