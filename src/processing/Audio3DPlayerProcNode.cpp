
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022-2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "Audio3DPlayerProcNode.hpp"

#include "cinder/gl/Fbo.h"
#include "cinder/audio/Param.h"

act::proc::Audio3DPlayerProcNode::Audio3DPlayerProcNode() : ProcNodeBase("Audio3DPlayer", NT_OUTPUT) {

	m_playPosition		= 0.0f;
	m_fadeInPosition	= 0.0f;
	m_fadeOutPosition	= 1.0f;
	m_length			= 0.0f;
	m_playSpeed			= 1.0f;
	m_volume			= 90.0f;
	m_toVolume			= 90.0f;
	m_isOpenDialog		= false;
	m_isPlaying			= false;
	m_isLooping			= false;
	m_noTimestretch		= true;

	m_falseCount		= 0;
	m_countFalseUpTo	= 1;

	m_3DPosition	= vec3(0.0f, 1.0f, 0.0f);

	m_drawSize		= ivec2(400, 150);

	addRPC("play", [&]() { return play(); });
	addRPC("stop", [&]() { return stop(); });
	
	auto trigger	= createBoolInput("fire",		[&](bool event)  { this->onTrigger(event); });
	auto gain		= createNumberInput("gain",		[&](float event)  { for(auto& node : m_soundRoomNodes) node->setVolume(audio::linearToDecibel(event)); });
	auto position	= createVec3Input("position",	[&](vec3 event)  { set3DPosition(event); });
	auto speed		= createNumberInput("speed",	[&](float event) { setPlaySpeed(event); });
	
	m_playPosPort	= createNumberOutput("playing at");
	m_bufferPort	= createAudioOutput("buffer");

	m_currentVolumePort = createNumberOutput("current volume");

	auto ctx = audio::Context::master();
	//ctx->disable();
}

act::proc::Audio3DPlayerProcNode::~Audio3DPlayerProcNode() {
	if(m_isPlaying) {
		for (auto& node : m_soundRoomNodes) {
			node->stop();
			node->disconnectExternals();
		}
	}
}

void act::proc::Audio3DPlayerProcNode::setup(act::room::RoomManagers roomMgrs) {
	m_audioMgr = roomMgrs.audioMgr;
}

void act::proc::Audio3DPlayerProcNode::init() {
	for(auto& path : m_paths) {
		loadSound(path);
		m_isAdding = true;
	}
	m_isAdding = false;
}

void act::proc::Audio3DPlayerProcNode::update() {
	if(m_isOpenDialog) {
		m_isOpenDialog = false;
		auto path = ci::app::getOpenFilePath();
		m_paths.push_back(path.string());
		loadSound(path);
	}

	if (!m_soundRoomNodes.empty()) {

		if (m_isPlaying) {
			m_playPosition = m_soundRoomNodes[0]->getPlayPosition();
			m_playPosPort->send(m_playPosition);
		};
	

		if (m_soundRoomNodes[0]->getBufferPlayer()->isEof()) {
			m_playPosition = 0.0f;
			for (auto& node : m_soundRoomNodes) node->getBufferPlayer()->seek(0.0f);
			if (!m_isLooping) {
				m_isPlaying = false;
			}
			else {
				for (auto& node : m_soundRoomNodes) node->play();
			};
				

		}

		for (auto& node : m_soundRoomNodes) node->update();

		m_currentVolumePort->send(m_soundRoomNodes[0]->getCurrentVolume());
		//m_currentVolumePort->send(std::clamp((m_soundRoomNode->getCurrentVolume() + 100) * 0.01f, 0.0f, 1.0f));

	}
}

void act::proc::Audio3DPlayerProcNode::draw() {
	beginNodeDraw();

	if(ImGui::Button("load")) {
		m_isOpenDialog = true;
	}
	if (m_soundRoomNodes.empty()) {
		endNodeDraw();
		return;
	}

	ImGui::SameLine();
	if (ImGui::Button("add")) {
		m_isAdding = true;
		m_isOpenDialog = true;
	}
	
	ImGui::SameLine();
	if (ImGui::Button("play")) {
		play();
	}

	ImGui::SameLine();
	if (ImGui::Button("stop")) {
		stop();
	}

	ImGui::SameLine();
	ImGui::Checkbox("collapsed", &m_isCollapsed);

	if (m_isCollapsed) {
		endNodeDraw();
		return;
	}

	bool prvntDrag = false;

	ImGui::SetNextItemWidth(m_drawSize.x);
	m_toVolume = m_soundRoomNodes[0]->getVolume();
	if (ImGui::SliderFloat("volume", &m_toVolume, 0.0f, 120.f)) {
		//m_gain->setValue(audio::decibelToLinear(m_volume));
		for (auto& node : m_soundRoomNodes) node->setVolume(m_toVolume);
		m_volume = m_toVolume;
		prvntDrag = true;
	}

	ImGui::SetNextItemWidth(m_drawSize.x);
	if (ImGui::SliderFloat("speed", &m_playSpeed, 0.0f, 6.f)) {
		setPlaySpeed(m_playSpeed);
		prvntDrag = true;
	}

	if (ImGui::Checkbox("noTimestretch", &m_noTimestretch)) {
		init();
	}

	if (ImGui::Checkbox("looping", &m_isLooping)) {
		for (auto& node : m_soundRoomNodes) node->loop(m_isLooping);
	}

	ImGui::SameLine();
	if (ImGui::Checkbox("waveform", &m_showWaveform)) {
			
	}

	std::string length = "length: "+ std::to_string(m_soundRoomNodes[0]->getSeconds());
	ImGui::Text(length.c_str());

		
	ImGui::PushID(1);
	//ImGui::BeginDisabled();
	if (ImGui::SliderFloat("", &m_playPosition, 0.0f, 1.0f)) {
		prvntDrag = true;
		for (auto& node : m_soundRoomNodes) node->getBufferPlayer()->seek(m_playPosition * m_soundRoomNodes[0]->getBufferPlayer()->getNumFrames());
	}
	//ImGui::EndDisabled();
	ImGui::PopID();

	ImGui::PushID(2);
	if (ImGui::SliderFloat("", &m_fadeInPosition, 0.0f, 1.0f)) {
		if (m_fadeInPosition > m_fadeOutPosition)
			m_fadeInPosition = m_fadeOutPosition;
		if (!m_soundRoomNodes.empty())
			for (auto& node : m_soundRoomNodes) node->setFadeIn(m_fadeInPosition);
		prvntDrag = true;
	}
	ImGui::PopID();

	ImGui::PushID(3);
	if (ImGui::SliderFloat("", &m_fadeOutPosition, 0.0f, 1.0f)) {
		if (m_fadeOutPosition < m_fadeInPosition)
			m_fadeOutPosition = m_fadeInPosition;
		if (m_soundRoomNodes[0])
			for (auto& node : m_soundRoomNodes) node->setFadeOut(m_fadeOutPosition);
		prvntDrag = true;
	}
	ImGui::PopID();

	preventDrag(prvntDrag);


	if (m_waveformTex && m_showWaveform)
		ImGui::Image(m_waveformTex, m_waveformTex->getSize(), vec2(0, 1), vec2(1, 0));

	ImGui::SetNextItemWidth(m_drawSize.x);
	//ImGui::InputInt("count Trigger", &m_countFalseUpTo, 0, 20);

	endNodeDraw();
}

void act::proc::Audio3DPlayerProcNode::onTrigger(bool event) {
	if (event)
		m_falseCount = 0;
	else {
		if(m_falseCount < m_countFalseUpTo)
			m_falseCount++;
	}
	
	if (event && !m_isPlaying) {
		//stop();
		//size_t positionFrames = floor(m_playPosition * m_soundRoomNode->getBufferPlayer()->getNumFrames());
		play();
		//m_soundRoomNode->getBufferPlayer()->seek(positionFrames);
		
		m_isPlaying = true;
	}
	else if (m_isPlaying && !event) {
		stop();
		m_isPlaying = false;
	}
}

bool act::proc::Audio3DPlayerProcNode::play()
{
	bool wasPlaying = m_isPlaying;
	for (auto& node : m_soundRoomNodes) node->play();
	m_isPlaying = true;
	return !wasPlaying;
}

bool act::proc::Audio3DPlayerProcNode::stop()
{
	bool wasPlaying = m_isPlaying;
	m_isPlaying = false;
	for (auto& node : m_soundRoomNodes) node->stop();
	//m_playPosition = 0.0f;
	return !wasPlaying;
}

void act::proc::Audio3DPlayerProcNode::setPlaySpeed(float speed)
{
	m_playSpeed = speed;
	for (auto& node : m_soundRoomNodes) node->setSpeed(speed);
}

void act::proc::Audio3DPlayerProcNode::loadSound(std::filesystem::path path) {
	
	if (path == "")
		return;

	try {
		auto soundNode = m_audioMgr->createSoundFile(m_3DPosition, path, 0.2f, path.filename().string(), m_noTimestretch);

		soundNode->setVolume(m_toVolume);
		soundNode->setFadeIn(m_fadeInPosition);
		soundNode->setFadeOut(m_fadeOutPosition);
		soundNode->loop(m_isLooping);

		soundNode->setPosition(m_3DPosition);

		m_soundRoomNodes.push_back(soundNode);

		auto waveform = WaveformPlot();
		auto buf = soundNode->getBufferPlayer()->getBuffer();
		waveform.load(buf, Rectf(vec2(0, 0), m_drawSize));

		gl::Fbo::Format format;
		format.setSamples(4);
		auto fbo = gl::Fbo::create(m_drawSize.x, m_drawSize.y, format);
		{
			gl::ScopedFramebuffer fbScp(fbo);
			gl::ScopedViewport scpVp(ivec2(0), fbo->getSize());

			gl::ScopedMatrices scpMatrices;
			gl::setMatricesWindow(fbo->getSize(), true);

			gl::clear(util::Design::backgroundColor());

			waveform.draw();
		}
		m_waveformTex = fbo->getColorTexture();
		m_length = soundNode->getSeconds();

		m_bufferPort->send(buf);
		
	} catch(...) {
		// it's not a sound
	}
	for (auto& node : m_soundRoomNodes) node->stop();
}

ci::Json act::proc::Audio3DPlayerProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["paths"]			= m_paths;
	json["isPlaying"]		= m_isPlaying;
	json["showWaveform"]	= m_showWaveform;
	json["hasChannelSplit"] = m_hasChannelSplit;
	json["isCollapsed"]		= m_isCollapsed;
	json["toVolume"]		= m_toVolume;
	json["volume"]			= m_volume.value();
	json["looping"]			= m_isLooping;
	json["fadeInPosition"]  = m_fadeInPosition;
	json["fadeOutPosition"] = m_fadeOutPosition;
	json["playPosition"]	= m_playPosition;
	json["length"]			= m_length;
	json["noTimestretch"]	= m_noTimestretch;
	
	ci::Json posJson = ci::Json::array();

	for (auto& node : m_soundRoomNodes) {
		vec3 pos = node->getPosition();
		auto j = ci::Json::object();
		j["x"] = pos[0];
		j["y"] = pos[1];
		j["z"] = pos[2];
		posJson.push_back(j);
	}
	json["positions"] = posJson; 	
	return json;
}

void act::proc::Audio3DPlayerProcNode::fromParams(ci::Json json) {
	// get new values for comparison
	bool shallPlay;
	float newX;
	float newY;
	float newZ;

	util::setValueFromJson(json, "looping", m_isLooping);
	util::setValueFromJson(json, "showWaveform", m_showWaveform);
	util::setValueFromJson(json, "isCollapsed", m_isCollapsed);
	util::setValueFromJson(json, "noTimestretch", m_noTimestretch);
	util::setValueFromJson(json, "hasChannelSplit", m_hasChannelSplit);

	if(json.contains("paths")) {
		m_paths.clear();
		for (auto& path : json["paths"])
			m_paths.push_back(path.get<std::string>());
		init();
	}
	if (util::setValueFromJson(json, "isPlaying", shallPlay)) {
		if (shallPlay != m_isPlaying) {
			onTrigger(true);
		};
		
	};

	if(json.contains("positions")) {
		for (size_t i = 0; i < json["positions"].size(); i++) {
			auto& posJson = json["positions"][i];
			if (i < m_soundRoomNodes.size()) {
				float x, y, z;
				util::setValueFromJson(posJson, "x", x);
				util::setValueFromJson(posJson, "y", y);
				util::setValueFromJson(posJson, "z", z);
				m_soundRoomNodes[i]->setPosition(vec3(x, y, z));
			}
		}
	}

	// check for position changes
	if (util::setValueFromJson(json, "x", newX)) {
		const vec3 newPosition = vec3(newX, m_3DPosition[1], m_3DPosition[2]);
		set3DPosition(newPosition);
	};
	if (util::setValueFromJson(json, "y", newY)) {
		const vec3 newPosition = vec3(m_3DPosition[0], newY, m_3DPosition[2]);
		set3DPosition(newPosition);
	};
	if (util::setValueFromJson(json, "z", newZ)) {
		const vec3 newPosition = vec3(m_3DPosition[0], m_3DPosition[1], newZ);
		set3DPosition(newPosition);
	};

	util::setValueFromJson(json, "toVolume", m_toVolume);
	if (util::setValueFromJson(json, "volume", m_volume.value())) {
		for (auto& node : m_soundRoomNodes) node->setVolume(m_volume.value());
	};
	if (util::setValueFromJson(json, "fadeInPosition", m_fadeInPosition)) {
		for (auto& node : m_soundRoomNodes) node->setFadeIn(m_fadeInPosition);
	};
	if (util::setValueFromJson(json, "fadeOutPosition", m_fadeOutPosition)) {
		for (auto& node : m_soundRoomNodes) node->setFadeOut(m_fadeOutPosition);
	};
}

void act::proc::Audio3DPlayerProcNode::set3DPosition(vec3 position)
{
	m_3DPosition = position;
	for (auto& node : m_soundRoomNodes) node->setPosition(position);
}
