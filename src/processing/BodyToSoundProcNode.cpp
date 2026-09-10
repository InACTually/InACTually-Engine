
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2025

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "BodyToSoundProcNode.hpp"


act::proc::BodyToSoundProcNode::BodyToSoundProcNode() : ProcNodeBase("BodyToSound") {

	m_drawSize = glm::ivec2(250, 250);

	auto body = InputPort<room::BodyRef>::create(PT_BODY, "Body", [&](room::BodyRef body) { this->onBody(body); });
	m_inputPorts.push_back(body);

	m_audioOutPort = createAudioNodeOutput("audioNode");
	m_localMovementPort = createNumberOutput("localMovement");
	m_globalMovementPort = createNumberOutput("globalMovement");

	m_localMovement = 0.0f;
	m_globalMovement = 0.0f;
	
	m_oldBody = nullptr;
	m_scaleValue = 1.0f;



	ci::audio::Context* ctx = ci::audio::master();

	m_osc		= ctx->makeNode(new ci::audio::GenTriangleNode);
	m_modFM		= ctx->makeNode(new ci::audio::GenSineNode);
	m_modAM		= ctx->makeNode(new ci::audio::GenSineNode);
	m_add		= ctx->makeNode(new ci::audio::AddNode);
	m_mul		= ctx->makeNode(new ci::audio::MultiplyNode);
	m_gain		= ctx->makeNode(new ci::audio::GainNode);
	m_gainFM	= ctx->makeNode(new ci::audio::GainNode);
	m_lowP		= ctx->makeNode(new ci::audio::FilterLowPassNode);

	//m_sine >> m_gain;

	m_osc->setFreq(220);
	m_modFM->setFreq(220);
	m_modAM->setFreq(20);
	m_add->setValue(200);
	m_mul->setValue(50);
	m_gain->setValue(1.0f);
	m_gainFM->setValue(200);



	m_modFM >> m_gainFM;
	m_osc->getParamFreq()->setProcessor(m_gainFM);

	m_mul->getParam()->setProcessor(m_modAM);

	m_osc >> m_mul >> m_lowP >> m_gain;

	m_osc->enable();
	m_modFM->enable();
	m_modAM->enable();
	m_gain->enable();


	m_audioOutPort->send(m_gain);
}

act::proc::BodyToSoundProcNode::~BodyToSoundProcNode() {
}

void act::proc::BodyToSoundProcNode::onBody(room::BodyRef body)
{
	if (m_oldBody == nullptr) {
		m_oldBody = body;
		return;
	}

	m_localMovement			= calcLocalMovement(body) * m_scaleValue;
	m_globalMovement		= calcGlobalMovement(body) * m_scaleValue;
	m_handDistance			= calcHandDistance(body) * m_scaleValue;
	m_leftHandVelocity		= calcVelocity(m_oldBody->joints[room::BJT_HAND_LEFT]->position, body->joints[room::BJT_HAND_LEFT]->position);
	m_rightHandVelocity		= calcVelocity(m_oldBody->joints[room::BJT_HAND_RIGHT]->position, body->joints[room::BJT_HAND_RIGHT]->position);
	m_leftHandDistance		= ci::distance(body->joints[room::BJT_HAND_LEFT]->position, body->joints[room::BJT_SPINE_CHEST]->position);
	m_leftHandDistanceY		= ci::distance(body->joints[room::BJT_HAND_LEFT]->position.y, body->joints[room::BJT_SPINE_CHEST]->position.y);
	m_leftHandDistanceXZ	= ci::distance2(glm::vec2(body->joints[room::BJT_HAND_LEFT]->position.x, body->joints[room::BJT_HAND_LEFT]->position.z), glm::vec2(body->joints[room::BJT_SPINE_CHEST]->position.x, body->joints[room::BJT_SPINE_CHEST]->position.z));
	m_rightHandDistance		= ci::distance(body->joints[room::BJT_HAND_RIGHT]->position, body->joints[room::BJT_SPINE_CHEST]->position);
	m_rightHandDistanceY	= ci::distance(body->joints[room::BJT_HAND_RIGHT]->position.y, body->joints[room::BJT_SPINE_CHEST]->position.y);
	m_rightHandDistanceXZ	= ci::distance2(glm::vec2(body->joints[room::BJT_HAND_RIGHT]->position.x, body->joints[room::BJT_HAND_RIGHT]->position.z), glm::vec2(body->joints[room::BJT_SPINE_CHEST]->position.x, body->joints[room::BJT_SPINE_CHEST]->position.z));
	m_handDistanceY			= ci::distance(body->joints[room::BJT_HAND_LEFT]->position.y, body->joints[room::BJT_HAND_RIGHT]->position.y);
	m_handDistanceXZ		= ci::distance(glm::vec2(body->joints[room::BJT_HAND_LEFT]->position.x, body->joints[room::BJT_HAND_LEFT]->position.z), glm::vec2(body->joints[room::BJT_HAND_RIGHT]->position.x, body->joints[room::BJT_HAND_RIGHT]->position.z));
	
	m_pelvisKneeDistanceY   = ci::distance(body->joints[room::BJT_PELVIS]->position.y, body->joints[room::BJT_KNEE_RIGHT]->position.y);

	m_localMovementPort->send(m_localMovement);
	m_globalMovementPort->send(m_globalMovement);

	m_freqAM	= m_handDistance * 75 + 1;
	m_cutOff	= std::clamp(std::clamp(1.0f-(m_leftHandDistanceY * 1.5f), 0.0f, 1.0f) * 15000, 0.0f, 20000.0f)  + 50.0f;
	m_freqFM	= (m_rightHandDistanceXZ) * 2 * 150;

	m_modStrength = std::clamp(std::clamp((m_leftHandVelocity + m_leftHandVelocity), 0.0f, 1.0f) * 150 + (m_leftHandDistanceXZ * 2500) + 30, 0.0f, 1500.0f);

	m_volume = std::clamp((m_pelvisKneeDistanceY * 2), 0.0f, 1.0f);

	m_Q = 8;

	m_oldBody = body;
}


float act::proc::BodyToSoundProcNode::calcLocalMovement(room::BodyRef body) {
	if (m_oldBody == nullptr) {
		return 0;
	}

	int numJoints = body->joints.size();

	float totalDist = 0.0f;
	float distX, distY, distZ;

	for (int i = 0; i < numJoints; i++){


		glm::vec3 currJointPos = body->joints[i]->position;
		glm::vec3 oldJointPos = m_oldBody->joints[i]->position;

		distX = abs(currJointPos.x - oldJointPos.x);
		distY = abs(currJointPos.y - oldJointPos.y);
		distZ = abs(currJointPos.z - oldJointPos.z);

		totalDist += distX + distY + distZ;
	}

	if (totalDist == 0) {
		return m_localMovement/m_scaleValue;
	}

	return totalDist/(float)numJoints;
}

float act::proc::BodyToSoundProcNode::calcGlobalMovement(room::BodyRef body) {
	if (m_oldBody == nullptr){
		return 0;
	}

	float totalDist = 0.0f;
	float distX, distY, distZ;

	glm::vec3 currJointPos = body->joints[room::BJT_SPINE_CHEST]->position;
	glm::vec3 oldJointPos = m_oldBody->joints[room::BJT_SPINE_CHEST]->position;

	distX = abs(currJointPos.x - oldJointPos.x);
	distY = abs(currJointPos.y - oldJointPos.y);
	distZ = abs(currJointPos.z - oldJointPos.z);


	if (distX + distY + distZ == 0) {
		return m_globalMovement / m_scaleValue;
	}


	return distX + distY + distZ;
}

float act::proc::BodyToSoundProcNode::calcHandDistance(room::BodyRef body)
{
	auto left = body->joints[room::BJT_HAND_LEFT]->position;
	auto right = body->joints[room::BJT_HAND_RIGHT]->position;

	return ci::distance2(left, right);
}

float act::proc::BodyToSoundProcNode::calcVelocity(glm::vec3 last, glm::vec3 current)
{
	return ci::distance(last, current) * 10.f;
}

void act::proc::BodyToSoundProcNode::update() {
	m_modAM->setFreq(m_freqAM);
	m_modFM->setFreq(m_freqFM);
	m_gainFM->setValue(m_modStrength);

	m_lowP->setCutoffFreq(m_cutOff);
	m_lowP->setQ(m_Q);

	m_gain->setValue(m_volume);
}

void act::proc::BodyToSoundProcNode::draw() {
	beginNodeDraw();

	ImGui::SetNextItemWidth(m_drawSize.x - 50);

	ImGui::Value("local movement: ", m_localMovement, "%.3f");
	ImGui::Value("global movement: ", m_globalMovement, "%.3f");

	ImGui::SetNextItemWidth(m_drawSize.x * 2);
	preventDrag(ImGui::SliderFloat("scale value", &m_scaleValue, 0.1f, 5.0f));

	preventDrag(ImGui::SliderFloat("m_handDistance value", &m_handDistance, 0.1f, 5.0f));
	preventDrag(ImGui::SliderFloat("m_leftHandVelocity value", &m_leftHandVelocity, 0.1f, 5.0f));
	preventDrag(ImGui::SliderFloat("m_rightHandVelocity value", &m_rightHandVelocity, 0.1f, 5.0f));
	preventDrag(ImGui::SliderFloat("m_leftHandDistance value", &m_leftHandDistance, 0.1f, 5.0f));
	preventDrag(ImGui::SliderFloat("m_leftHandDistanceY value", &m_leftHandDistanceY, 0.1f, 5.0f));
	preventDrag(ImGui::SliderFloat("m_leftHandDistanceXZ value", &m_leftHandDistanceXZ, 0.1f, 5.0f));
	preventDrag(ImGui::SliderFloat("m_rightHandDistance value", &m_rightHandDistance, 0.1f, 5.0f));
	preventDrag(ImGui::SliderFloat("m_rightHandDistanceY value", &m_rightHandDistanceY, 0.1f, 5.0f));
	preventDrag(ImGui::SliderFloat("m_rightHandDistanceXZ value", &m_rightHandDistanceXZ, 0.1f, 5.0f));
	preventDrag(ImGui::SliderFloat("m_handDistanceY value", &m_handDistanceY, 0.1f, 5.0f));
	preventDrag(ImGui::SliderFloat("m_handDistanceXZ value", &m_handDistanceXZ, 0.1f, 5.0f));

	preventDrag(ImGui::SliderFloat("m_volume value", &m_volume, 0.0f, 1.0f));
	preventDrag(ImGui::SliderInt("m_cutOff value", &m_cutOff, 0, 15000));
	preventDrag(ImGui::SliderInt("m_Q value", &m_Q, 1, 10));
	preventDrag(ImGui::SliderInt("m_freqAM value", &m_freqAM, 0, 150));
	preventDrag(ImGui::SliderInt("m_freqFM value", &m_freqFM, 0, 150));
	preventDrag(ImGui::SliderInt("m_modStrength value", &m_modStrength, 0, 1500));

	endNodeDraw();
}

void act::proc::BodyToSoundProcNode::setup(act::room::RoomManagersRef rmgr)
{
	
}

ci::Json act::proc::BodyToSoundProcNode::toParams() {
	ci::Json json = ci::Json::object();
 
	return json; 
}

void act::proc::BodyToSoundProcNode::fromParams(ci::Json json) {
}
