
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
#include "MicrophoneProcNode.hpp"

act::proc::MicrophoneProcNode::MicrophoneProcNode() : ProcNodeBase("Microphone", NT_INPUT) {
	
	m_drawSize = ivec2(400, 300);

	m_show = false;

	m_selectedInput = 0;

	
	m_audioNodeOutPort = createAudioNodeOutput("inputNode");

	m_audioNodeOutPort->setConnectionCB([&]() {
		if(m_microphone)
			m_audioNodeOutPort->send(m_microphone->getOut());
	});
}

act::proc::MicrophoneProcNode::~MicrophoneProcNode() {
	
}

void act::proc::MicrophoneProcNode::setup(act::room::RoomManagers roomMgrs) {
	m_audioMgr = roomMgrs.audioMgr;

	auto mic = m_audioMgr->getMicrophoneByIndex(m_selectedInput);
	if (mic) {
		attachMic(mic);
	}
}

void act::proc::MicrophoneProcNode::update() {

}

void act::proc::MicrophoneProcNode::draw() {
	beginNodeDraw();

	if (m_audioMgr->getMicrophoneNames().empty())
		ImGui::Text("No Microphone has been set up.");
	else {
		ImGui::SetNextItemWidth(m_drawSize.x - ImGui::CalcTextSize("CameraDevice").x);
		if (ImGui::Combo("CameraDevice", &m_selectedInput, m_audioMgr->getMicrophoneNames())) {
			attachMic(m_audioMgr->getMicrophoneByIndex(m_selectedInput));
		}
	}


	if (ImGui::Checkbox("show", &m_show)) {
		// checkbox was clicked
	}

	if (m_show) {
		gl::pushMatrices();

		gl::pushMatrices();
	}

	endNodeDraw();
}



ci::Json act::proc::MicrophoneProcNode::toParams() {
	ci::Json json = ci::Json::object();
	UID uid = "";
	if (m_microphone)
		uid = m_microphone->getUID();
	json["selectedMicUID"] = uid;
	return json;
}

void act::proc::MicrophoneProcNode::fromParams(ci::Json json) {
	UID uid = "";
	if (util::setValueFromJson(json, "selectedDevice", uid)) {
		attachMic(m_audioMgr->getMicrophoneByUID(uid));
	}
}

void act::proc::MicrophoneProcNode::attachMic(room::MicrophoneRoomNodeRef mic)
{
	if (m_microphone) {
		m_microphone->disconnectExternals();
	}
	
	if (mic) {
		m_microphone = mic;
		m_audioNodeOutPort->send(m_microphone->getOut());
	}
}
