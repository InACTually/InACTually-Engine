
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
#include "TriggerListProcNode.hpp"
#include "cinder/rand.h"


act::proc::TriggerListProcNode::TriggerListProcNode() : ProcNodeBase("TriggerList") {
	m_drawSize = ivec2(500, 200);

	init = true;


	m_fireInputPort = createBoolInput("fire", [&](bool triggered) { 
		if (triggered && !m_isTriggering) {
			fireTrigger(); 
			m_isTriggering = false;
		}
		else if (!triggered && m_isTriggering) {
			m_isTriggering = true;
		}
	});

	m_stepUpInputPort = createBoolInput("step up", [&](bool triggered) { if (triggered) stepUp(); });
	m_stepDownInputPort = createBoolInput("step down", [&](bool triggered){ if (triggered) stepDown(); });
	m_resetInputPort = createBoolInput("reset", [&](bool triggered) { if (triggered) reset(); });
}

act::proc::TriggerListProcNode::~TriggerListProcNode() {
}

void act::proc::TriggerListProcNode::update() {
	if (init) {
		init = false;
	}
}

void act::proc::TriggerListProcNode::draw() {
	beginNodeDraw();

	ImGui::Dummy(ImVec2(m_drawSize.x, 0.0f));

	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, 100.0f);
	ImGui::SetColumnWidth(1, 400.0f);
	
	ImVec2 btnWidth = ImVec2(ImGui::GetContentRegionAvail().x - 5.0f, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));

	if (ImGui::Button(ICON_FA_PLUS_CIRCLE " add", btnWidth)) {
		addTrigger(0);
	}


	m_fireInputPort->draw(0, true);
	if (ImGui::Button(ICON_FA_ARROW_ALT_CIRCLE_RIGHT " fire", btnWidth)) {
		fireTrigger();
	}
	
	m_resetInputPort->draw(0, true);
	if (ImGui::Button(ICON_FA_ARROW_ALT_CIRCLE_LEFT " reset", btnWidth)) {
		reset();
	}

	m_stepUpInputPort->draw(0, true);
	if (ImGui::Button(ICON_FA_CHEVRON_CIRCLE_UP " up", btnWidth)) {
		stepUp();
	}

	m_stepDownInputPort->draw(0, true);
	if (ImGui::Button(ICON_FA_CHEVRON_CIRCLE_DOWN " down", btnWidth)) {
		stepDown();
	}

	ImGui::PopStyleVar();

	ImGui::NextColumn();

	for (int i = 0; i < m_outputPorts.size(); i++) {
		auto port = m_outputPorts[i];

		ImGui::PushID(i);
		
		
		if (ImGui::Button(ICON_FA_MINUS)) {
			removeTrigger(i);
		}
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_PLUS)) {
			addTrigger(i + 1);
		}
		ImGui::SameLine(); 
		if (ImGui::Button(ICON_FA_ARROW_UP)) {
			moveTriggerUp(i);
		}
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_ARROW_DOWN)) {
			moveTriggerDown(i);
		}

		ImGui::SameLine();
		if (m_nextTrigger == i) {
			ImGui::PushStyleColor(ImGuiCol_Text, util::Design::primaryColor());
			ImGui::Button(ICON_FA_CHEVRON_CIRCLE_RIGHT);
			ImGui::PopStyleColor();
		}
		else {
			if (ImGui::Button(ICON_FA_CIRCLE)) {
				setNextTrigger(i);
			}
		}

		ImGui::SameLine();
		std::string caption = port->getCaption();
		if (ImGui::InputText("", &caption)) {
			port->setCaption(caption);
		}

		port->draw(0, true);

		ImGui::PopID();
	}

	ImGui::Columns();

	endNodeDraw(false, false);
}

ci::Json act::proc::TriggerListProcNode::toParams() {
	ci::Json json = ci::Json::object();
	auto trigger = ci::Json::array();
	for (auto&& port : m_outputPorts) {
		auto portJson = ci::Json::object();

		portJson["name"] = port->getName();
		portJson["caption"] = port->getCaption();

		trigger.push_back(portJson);
	}
	json["trigger"] = trigger;
	json["startAtRandom"] = m_startAtRandom;
	return json;
}

void act::proc::TriggerListProcNode::fromParams(ci::Json json) {
	for (auto&& t : json["trigger"]) {
		std::string name = t["name"];
		std::string caption = t["caption"];

		auto port = createBoolOutput( name);
		port->setCaption(caption);
	}
	util::setValueFromJson(json, "startAtRandom", m_startAtRandom);
	if(m_startAtRandom && m_outputPorts.size() > 1) {
		m_nextTrigger = ci::randInt(m_outputPorts.size()-1);
	}
}

void act::proc::TriggerListProcNode::fireTrigger()
{
	auto port = std::dynamic_pointer_cast<OutputPort<bool>>(m_outputPorts[m_nextTrigger]);
	stepDown();
	if (port)
		port->send(true);
	if (m_nextTrigger == m_outputPorts.size() - 1)
		port->send(false);
}

void act::proc::TriggerListProcNode::stepUp()
{
	m_nextTrigger = m_nextTrigger - 1;
	if (m_nextTrigger < 0)
		m_nextTrigger = 0;
}

void act::proc::TriggerListProcNode::stepDown()
{
	m_nextTrigger = m_nextTrigger + 1;
	if (m_nextTrigger > m_outputPorts.size() - 1)
		m_nextTrigger = m_outputPorts.size() - 1;
}

void act::proc::TriggerListProcNode::setNextTrigger(int index)
{
	if (index >= m_outputPorts.size())
		return;

	m_nextTrigger = index;
}

void act::proc::TriggerListProcNode::reset()
{
	m_nextTrigger = 0;
}

void act::proc::TriggerListProcNode::addTrigger(int index)
{
	auto port = createBoolOutput("Trigger" + UniqueIDBase().getUID(), false);
	port->setCaption("new Trigger");
	
	if (index >= 0 && index < m_outputPorts.size())
		m_outputPorts.insert(m_outputPorts.begin() + index, port);
	else
		m_outputPorts.push_back(port);
}

void act::proc::TriggerListProcNode::setTriggerLabel(int index, std::string label)
{
	if (index >= m_outputPorts.size())
		return;

	m_outputPorts[index]->setCaption(label);
}

void act::proc::TriggerListProcNode::removeTrigger(int index)
{
	if (index >= m_outputPorts.size())
		return;

	m_outputPorts.erase(m_outputPorts.begin() + index);

	if (m_nextTrigger > m_outputPorts.size())
		m_nextTrigger = m_outputPorts.size();
}

void act::proc::TriggerListProcNode::moveTriggerUp(int index)
{
	if (index <= 0)
		return;

	moveTriggerTo(index, index - 1);
}

void act::proc::TriggerListProcNode::moveTriggerDown(int index)
{
	if (index >= m_outputPorts.size())
		return;

	moveTriggerTo(index, index + 1);
}

void act::proc::TriggerListProcNode::moveTriggerTo(int index, int to)
{
	if (index > to)
		std::rotate(m_outputPorts.rend() - index - 1, m_outputPorts.rend() - index, m_outputPorts.rend() - to);
	else
		std::rotate(m_outputPorts.begin() + index, m_outputPorts.begin() + index + 1, m_outputPorts.begin() + to + 1);
}
