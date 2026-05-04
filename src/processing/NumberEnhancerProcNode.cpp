
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
#include "NumberEnhancerProcNode.hpp"

#include <numeric>


act::proc::NumberEnhancerProcNode::NumberEnhancerProcNode() : ProcNodeBase("NumberEnhancer") {
	m_drawSize = ivec2(300, 300);

	auto number = createNumberInput("number", [&](float num) { this->onNumber(num); });
	
	m_numberPort = createNumberOutput("enhanced number");
	
	m_historySize = 1;
	m_history.resize(10, 0.0f);

	m_rawHistory.resize(20,0.0f);

	m_currentMin = 0.0f;
	m_currentMax = 1.0f;
	m_mappedMin = 0.0f;
	m_mappedMax = 1.0f;

	m_doGate = true;
	m_doMap = false;

	m_rawInputMin = 0;
	m_rawInputMax = 0;
}

act::proc::NumberEnhancerProcNode::~NumberEnhancerProcNode() {
}

void act::proc::NumberEnhancerProcNode::update() { 
}

void act::proc::NumberEnhancerProcNode::draw() {
	beginNodeDraw();
 
	ImGui::Value("mean number", m_meanNumber);
	ImGui::Value("raw input min", m_rawInputMin);
	ImGui::Value("raw input max", m_rawInputMax);

	auto rawVec = std::vector<float>(m_rawHistory.begin(), m_rawHistory.end());
	ImGui::PlotLines("Raw Input", rawVec.data(), rawVec.size());



	bool prvntDrag = false;

	prvntDrag = ImGui::DragInt("Filter History", &m_historySize, 1, 1);

	auto vec = std::vector<float>(m_history.begin(), m_history.end());
	ImGui::PlotLines("History Vis", vec.data(), vec.size());


	ImGui::Checkbox("Gate", &m_doGate);
	if(m_doGate) {
 		if (ImGui::InputFloat("min", &m_currentMin)) {
			prvntDrag = true;
			if (m_currentMin > m_currentMax)
				m_currentMin = m_currentMax;
		}
 		if (ImGui::InputFloat("max", &m_currentMax)) {
			prvntDrag = true;
			if (m_currentMin > m_currentMax)
				m_currentMax = m_currentMin;
		}

		ImGui::Checkbox("Mapper", &m_doMap);
		if (m_doMap) {
			if (ImGui::InputFloat("new min", &m_mappedMin)) {
				prvntDrag = true;
				if (m_mappedMin > m_mappedMax)
					m_mappedMin = m_mappedMax;
			}
			if (ImGui::InputFloat("new max", &m_mappedMax)) {
				prvntDrag = true;
				if (m_mappedMin > m_mappedMax)
					m_mappedMax = m_mappedMin;
			}
		}
	}
	
	endNodeDraw();
}

ci::Json act::proc::NumberEnhancerProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["historySize"]	= m_historySize;
	json["currentMin"]	= m_currentMin;
	json["currentMax"]	= m_currentMax;
	json["mappedMin"]	= m_mappedMin;
	json["mappedMax"]	= m_mappedMax;
	json["doGate"]		= m_doGate;
	json["doMap"]		= m_doMap;
	 
	return json;
}

void act::proc::NumberEnhancerProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "historySize", m_historySize);
	util::setValueFromJson(json, "currentMin", m_currentMin);
	util::setValueFromJson(json, "currentMax", m_currentMax);
	util::setValueFromJson(json, "mappedMin", m_mappedMin);
	util::setValueFromJson(json, "mappedMax", m_mappedMax);
	util::setValueFromJson(json, "doGate", m_doGate);
	util::setValueFromJson(json, "doMap", m_doMap);
}

void act::proc::NumberEnhancerProcNode::onNumber(float number) {

	m_rawHistory.push_back(number);
	if (m_rawHistory.size() > 20) {
		m_rawHistory.pop_front();
	}

	auto min_max = std::minmax_element(m_rawHistory.begin(), m_rawHistory.end());

	m_rawInputMin = *min_max.first;
	m_rawInputMax = *min_max.second;

	if (m_doGate) {

		number = std::min(m_currentMax, number);
		number = std::max(m_currentMin, number);

		if (m_doMap) {
			// normalized value [0,1]
			float currentRange = m_currentMax - m_currentMin;
			number = (number - m_currentMin) / currentRange;

			float mappedRange = m_mappedMax - m_mappedMin;
			number = number * mappedRange + m_mappedMin;
		}	
	}

	m_history.push_back(number);
	while (m_history.size() > m_historySize + 10)
		m_history.pop_front();

	m_meanNumber = std::accumulate(m_history.begin() + 10, m_history.end(), 0.0f) / (m_history.size() - 10);	
	m_numberPort->send(m_meanNumber); 
}
