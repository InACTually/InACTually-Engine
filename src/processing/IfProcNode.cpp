
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021�2025 Lars Engeln, Fabian T�pfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "IfProcNode.hpp"

#include <numeric>


act::proc::IfProcNode::IfProcNode() : ProcNodeBase("If") {
	m_drawSize = ivec2(300, 300);

	auto number = InputPort<float>::create(PT_NUMBER, "number", [&](float num) { this->onNumber(num); });
	auto text = InputPort<std::string>::create(PT_TEXT, "text", [&](std::string text) { this->onText(text); });
	auto feat = InputPort<feature>::create(PT_FEATURE, "feature", [&](feature f) { this->onFeature(f); });
	auto featList = InputPort<featureList>::create(PT_FEATURELIST, "feature list", [&](featureList fl) { this->onFeatures(fl); });
	auto vecCompare = InputPort<vec3>::create(PT_VEC3, "vec3", [&](vec3 pos) {this->onVec(pos); });
	m_inputPorts.push_back(number);
	m_inputPorts.push_back(text);
	m_inputPorts.push_back(feat);
	m_inputPorts.push_back(featList);
	m_inputPorts.push_back(vecCompare);
	
	m_resultPort = OutputPort<bool>::create(PT_BOOL, "result");
	m_outputPorts.push_back(m_resultPort);
	m_resultPort_neg = OutputPort<bool>::create(PT_BOOL, "result negated");
	m_outputPorts.push_back(m_resultPort_neg);
	
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
	ImGui::InputFloat("Radius", &precision);
	ImGui::Checkbox("not", &m_not);
	
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
	json["radius"]					= precision;
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
	util::setValueFromJson(json, "radius", precision);
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

void act::proc::IfProcNode::onNumber(float number) {
	m_resultPort->send(compareValue(number));
}

void act::proc::IfProcNode::onText(std::string text) {
	m_resultPort->send(compareText(text));
}

void act::proc::IfProcNode::onFeature(act::proc::feature f) {
	m_resultPort->send(compareText(f.first) && compareValue(f.second));
}

void act::proc::IfProcNode::onFeatures(act::proc::featureList fList) {
	bool result = false;
	for(auto&& f : fList) {
		if(compareText(f.first) && compareValue(f.second)) {
			result = true;
			break;
		}
	}
	m_resultPort->send(result);
}
bool act::proc::IfProcNode::compareVec(vec3 pos)
{
	float x = abs(pos.x - m_compareVec3[0]);
	float y = abs(pos.y - m_compareVec3[1]);
	float z = abs(pos.z - m_compareVec3[2]);
	if(x<precision && y<precision && z<precision)
	{
		return true;
	}
	return false;
}

void act::proc::IfProcNode::onVec(vec3 pos)
{
	bool res = compareVec(pos);
	m_resultPort->send(res);
	if (res) {
		m_resultPort_neg->send(false);
	}
	

}