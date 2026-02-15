
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021, 2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "IfProcNode.hpp"

#include <numeric>


act::proc::IfProcNode::IfProcNode() : ProcNodeBase("If") {
	m_drawSize = ivec2(300, 300);

	auto number = createNumberInput("number", [&](float num) { this->onNumber(num); });
	auto text = createTextInput("text", [&](std::string text) { this->onText(text); });
	auto feat = createFeatureInput("feature", [&](feature f) { this->onFeature(f); });
	auto featList = createFeatureListInput("feature list", [&](featureList fl) { this->onFeatures(fl); });
	auto vecCompare = createVec3Input("vec3", [&](vec3 pos) {this->onVec(pos); });

	
	m_resultPort = createBoolOutput("result");
	m_resultPort_neg = createBoolOutput("result negated");
	
	m_compareOptions.push_back("==");
	m_compareOptions.push_back("!=");
	m_compareOptions.push_back("<");
	m_compareOptions.push_back(">");
	m_compareOptions.push_back("<=");
	m_compareOptions.push_back(">=");
	m_compareOptionSelection = 1;
	m_compareValue = 0.0f;
	
	m_compareText = "text";
	m_not = false;
}

act::proc::IfProcNode::~IfProcNode() {
}

void act::proc::IfProcNode::update() {
}

void act::proc::IfProcNode::draw() {
	beginNodeDraw();

	ImGui::InputFloat("compare value", &m_compareValue);
	ImGui::Combo("compare method", &m_compareOptionSelection, m_compareOptions);

	ImGui::InputText("compare text", &m_compareText);
	ImGui::InputFloat3("vec3", m_compareVec3);
	ImGui::SameLine();
	ImGui::InputFloat("Radius", &m_precision);
	ImGui::Checkbox("not", &m_not);
	ImGui::Checkbox("fireOnlyOnce", &m_fireOnlyOnce);
	
	endNodeDraw();
}

ci::Json act::proc::IfProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["compareValue"]			= m_compareValue;
	json["compareText"]				= m_compareText;
	json["compareOptionSelection"]	= m_compareOptionSelection;
	json["not"]						= m_not;
	json["pos_x"]					= m_compareVec3[0];
	json["pos_y"]					= m_compareVec3[1];
	json["pos_z"]					= m_compareVec3[2];
	json["radius"]					= m_precision;
	json["fireOnlyOnce"]			= m_fireOnlyOnce;
	return json;
}

void act::proc::IfProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "compareValue", m_compareValue);
	util::setValueFromJson(json, "compareText", m_compareText);
	util::setValueFromJson(json, "compareOptionSelection", m_compareOptionSelection);
	util::setValueFromJson(json, "not", m_not);
	util::setValueFromJson(json, "pos_x", m_compareVec3[0]); 
	util::setValueFromJson(json, "pos_y", m_compareVec3[1]); 
	util::setValueFromJson(json, "pos_z", m_compareVec3[2]);
	util::setValueFromJson(json, "pos_z", m_compareVec3[2]);
	util::setValueFromJson(json, "radius", m_precision);
	util::setValueFromJson(json, "fireOnlyOnce", m_fireOnlyOnce);
}

bool act::proc::IfProcNode::compareValue(float number) {
	bool result = false;
	switch(m_compareOptionSelection) {
	case 0: // ==
		result = number == m_compareValue;
		break;
	case 1: // !=
		result = number != m_compareValue;
		break;
	case 2: // <
		result = number < m_compareValue;
		break;
	case 3: // >
		result = number > m_compareValue;
		break;
	case 4: // <=
		result = number <= m_compareValue;
		break;
	case 5: // >=
		result = number >= m_compareValue;
		break;
	}
	return result;
}

bool act::proc::IfProcNode::compareText(std::string text) {
	bool result = text == m_compareText;
	if (result && !m_not)
		return true;
	if (!result && m_not)
		return true;
	return false;
}

bool act::proc::IfProcNode::compareVec(vec3 pos)
{
	float x = abs(pos.x - m_compareVec3[0]);
	float y = abs(pos.y - m_compareVec3[1]);
	float z = abs(pos.z - m_compareVec3[2]);
	if (x < m_precision && y < m_precision && z < m_precision)
	{
		return true;
	}
	return false;
}

void act::proc::IfProcNode::sendResult(bool val)
{
	if (m_fireOnlyOnce) {
		if (val != m_lastCompareResult) {
			m_resultPort->send(val);
			m_resultPort_neg->send(!val);
		}
	}
	else {
		m_resultPort->send(val);
		m_resultPort_neg->send(!val);
	}
	m_lastCompareResult = val;
}

void act::proc::IfProcNode::onNumber(float number) {
	bool val = compareValue(number);
	sendResult(val);
}

void act::proc::IfProcNode::onText(std::string text) {
	bool val = compareText(text);
	sendResult(val);
}

void act::proc::IfProcNode::onFeature(act::proc::feature f) {
	bool val = compareText(f.first) && compareValue(f.second);
	sendResult(val);
}

void act::proc::IfProcNode::onFeatures(act::proc::featureList fList) {
	bool val = false;
	for(auto&& f : fList) {
		if(compareText(f.first) && compareValue(f.second)) {
			val = true;
			break;
		}
	}
	sendResult(val);
}

void act::proc::IfProcNode::onVec(vec3 pos)
{
	bool val = compareVec(pos);
	sendResult(val);
}