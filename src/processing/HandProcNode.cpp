
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021�2025 Lars Engeln, Fabian T�pfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "HandProcNode.hpp"


act::proc::HandProcNode::HandProcNode() : ProcNodeBase("Hand") {
	m_drawSize = ivec2(250, 250);

	m_leftHandClosed = OutputPort<bool>::create(PT_BOOL, "is left closed");
	m_leftHandOpend = OutputPort<bool>::create(PT_BOOL, "is left opened");
	m_rightHandClosed = OutputPort<bool>::create(PT_BOOL, "is right closed");
	m_rightHandOpend = OutputPort<bool>::create(PT_BOOL, "is right opened");
	//m_orientation = OutputPort<ci::quat>::create(PT_QUAT, "orientation");
	m_outputPorts.push_back(m_leftHandClosed);
	m_outputPorts.push_back(m_leftHandOpend);
	m_outputPorts.push_back(m_rightHandClosed);
	m_outputPorts.push_back(m_rightHandOpend);
	//m_outputPorts.push_back(m_orientation);

	auto skeleton = InputPort<std::tuple<uint32_t, k4abt_skeleton_t>>::create(PT_BODY, "Skeleton", [&](std::tuple<uint32_t, k4abt_skeleton_t> skeletonTuple) { onBody(std::get<1>(skeletonTuple)); });
	m_inputPorts.push_back(skeleton);
}

act::proc::HandProcNode::~HandProcNode() {
}

void act::proc::HandProcNode::setup() {
}

void act::proc::HandProcNode::update() {
}

void act::proc::HandProcNode::draw() {
	beginNodeDraw();

	{
		std::stringstream txt;
		txt << "left:  " << m_distanceLeft;
		ImGui::Text(txt.str().c_str());
	}
	{
		std::stringstream txt;
		txt << "right: " << m_distanceRight;
		ImGui::Text(txt.str().c_str());
	}

	endNodeDraw();
}

void act::proc::HandProcNode::onBody(k4abt_skeleton_t body)
{
	auto leftTip	= body.joints[K4ABT_JOINT_HANDTIP_LEFT].position.xyz;
	auto rightTip	= body.joints[K4ABT_JOINT_HANDTIP_RIGHT].position.xyz;
	auto leftWrist	= body.joints[K4ABT_JOINT_WRIST_LEFT].position.xyz;
	auto rightWrist = body.joints[K4ABT_JOINT_WRIST_RIGHT].position.xyz;
	auto leftHand	= body.joints[room::BJT_HAND_LEFT].position.xyz;

	//m_distanceLeft	= ci::dot(vec3(leftTip.x, leftTip.y, leftTip.z) - vec3(leftHand.x, leftHand.y, leftHand.z), vec3(leftWrist.x, leftWrist.y, leftWrist.z) - vec3(leftHand.x, leftHand.y, leftHand.z));
	m_distanceRight = ci::distance(vec3(rightTip.x, rightTip.y, rightTip.z), vec3(rightWrist.x, rightWrist.y, rightWrist.z));
	m_distanceLeft = ci::distance(vec3(leftTip.x, leftTip.y, leftTip.z), vec3(leftWrist.x, leftWrist.y, leftWrist.z));
	
	if (m_distanceLeft < 0.2f) m_leftHandClosed->send(true);
	else m_leftHandOpend->send(true);
	if (m_distanceRight < 0.2f) m_rightHandClosed->send(true);
	else m_rightHandOpend->send(true);
}

ci::Json act::proc::HandProcNode::toParams() {
	ci::Json json = ci::Json::object();
	return json;
}

void act::proc::HandProcNode::fromParams(ci::Json json) {
}