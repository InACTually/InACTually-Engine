
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "DistributorProcNode.hpp"
#include "cinder\Rand.h"

act::proc::DistributorProcNode::DistributorProcNode() : ProcNodeBase("Distributor") {
	m_drawSize = ci::ivec2(200, 200);

	m_options.push_back("all");
	m_options.push_back("round robin");
	m_options.push_back("random");


	createBoolInput("in", [&](bool val) { distribute(val); });
	
}

act::proc::DistributorProcNode::~DistributorProcNode() {
}

void act::proc::DistributorProcNode::update() {
	
}

void act::proc::DistributorProcNode::draw() {
	beginNodeDraw();

	ImGui::Combo("option", &m_selectedOption, m_options);
	if (ImGui::InputInt("outputs", &m_numOfOutputs)) {
		if (m_numOfOutputs < 0)
			m_numOfOutputs = 0;
		updateOutputs();
	};

	endNodeDraw();
}


ci::Json act::proc::DistributorProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["selectedOption"] = m_selectedOption;
	json["numOfOutputs"] = m_numOfOutputs;
	return json;
}

void act::proc::DistributorProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "selectedOption", m_selectedOption);
	util::setValueFromJson(json, "numOfOutputs", m_numOfOutputs);

	updateOutputs();
}

void act::proc::DistributorProcNode::updateOutputs() {
	int currentOutputs = m_outputs.size();
	if (currentOutputs == m_numOfOutputs)
		return;

	if (currentOutputs > m_numOfOutputs) {
		while (m_outputs.size() > m_numOfOutputs) {
			removeOutputPort(m_outputs[m_outputs.size() - 1]);
			m_outputs.resize(m_outputs.size() - 1);
		}
	}
	else {
		for (int i = currentOutputs; i < m_numOfOutputs; i++) {
			std::stringstream strstr;
			strstr << i + 1;
			m_outputs.push_back(createBoolOutput(strstr.str()));
		}
	}
}

void act::proc::DistributorProcNode::distribute(bool val)
{
	if (m_outputs.empty())
		return;

	switch (m_selectedOption) {
	case DO_ALL:
		for (auto port : m_outputs)
			port->send(val);
		break;
	case DO_ROUNDROBIN:
		currentRRIndex++;
		if (m_outputs.size() <= currentRRIndex)
			currentRRIndex = 0;
		m_outputs[currentRRIndex]->send(val);
		break;
	case DO_RANDOM:
		m_outputs[ci::randInt(0, m_outputs.size())]->send(val);
		break;
	}
}
