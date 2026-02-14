
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022-2025

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "Audio3DPlayerTimestretchProcNode.hpp"

#include "cinder/gl/Fbo.h"
#include "cinder/audio/Param.h"

act::proc::Audio3DPlayerTimestretchProcNode::Audio3DPlayerTimestretchProcNode() : ProcNodeBase("Audio3DPlayer", NT_OUTPUT) {

	m_playPosition		= 0.0f;
	m_fadeInPosition	= 0.0f;
	m_fadeOutPosition	= 1.0f;
	m_length			= 0.0f;
	m_path				= "";
	m_playSpeed			= 1.0f;
	m_volume			= 90.0f;
	m_toVolume			= 90.0f;
	m_isOpenDialog		= false;
	m_isPlaying			= false;
	m_isLooping			= false;

	m_falseCount		= 0;
	m_countFalseUpTo	= 1;

	m_3DPosition	= vec3(0.0f, 1.0f, 0.0f);

	m_drawSize		= ivec2(400, 150);

	addRPC("play", [&]() { return play(); });
	addRPC("stop", [&]() { return stop(); });
	
	auto trigger	= createBoolInput(		"fire",		[&](bool event) { this->onTrigger(event); });
	auto gain		= createNumberInput(	"gain",		[&](float event) { m_soundRoomNode->setVolume(audio::linearToDecibel(event)); });
	auto position	= createVec3Input(		"position", [&](vec3 event) { set3DPosition(event); });
	auto speed		= createNumberInput(	"speed",	[&](float event) { setPlaySpeed(event); });

	m_playPosPort	= createNumberOutput("playing at");
	m_bufferPort	= createAudioOutput("buffer");

	m_currentVolumePort = createNumberOutput("current volume");

	auto ctx = audio::Context::master();
	//ctx->disable();
}

act::proc::Audio3DPlayerTimestretchProcNode::~Audio3DPlayerTimestretchProcNode() {
	if(m_isPlaying) {
		m_soundRoomNode->stop();
		m_soundRoomNode->disconnectExternals();
	}
}

void act::proc::Audio3DPlayerTimestretchProcNode::setup(act::room::RoomManagers roomMgrs) {
	m_audioMgr = roomMgrs.audioMgr;
}

void act::proc::Audio3DPlayerTimestretchProcNode::init() {
	loadSound(m_path);	
}

void act::proc::Audio3DPlayerTimestretchProcNode::update() {
	if(m_isOpenDialog) {
		m_isOpenDialog = false;
		auto path = ci::app::getOpenFilePath();
		m_path = path.string();
		loadSound(path);
	}

	if (m_soundRoomNode) {

		if (m_isPlaying) {
			m_playPosition = m_soundRoomNode->getPlayPosition();
			m_playPosPort->send(m_playPosition);
		};
	

		if (m_soundRoomNode->getBufferPlayer()->isEof()) {
			m_playPosition = 0.0f;
			m_soundRoomNode->getBufferPlayer()->seek(m_playPosition * m_soundRoomNode->getBufferPlayer()->getNumFrames());
			if (!m_isLooping) {
				m_isPlaying = false;
			}
			else {
				m_soundRoomNode->play();
			};
				

		}

		m_soundRoomNode->update();

		m_currentVolumePort->send(m_soundRoomNode->getCurrentVolume());
		//m_currentVolumePort->send(std::clamp((m_soundRoomNode->getCurrentVolume() + 100) * 0.01f, 0.0f, 1.0f));

	}
}

void act::proc::Audio3DPlayerTimestretchProcNode::draw() {
	beginNodeDraw();

	if(ImGui::Button("load")) {
		m_isOpenDialog = true;
	}
	if (!m_soundRoomNode) {
		endNodeDraw();
		return;
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
	m_toVolume = m_soundRoomNode->getVolume();
	if (ImGui::SliderFloat("volume", &m_toVolume, 0.0f, 120.f)) {
		//m_gain->setValue(audio::decibelToLinear(m_volume));
		m_soundRoomNode->setVolume(m_toVolume);
		m_volume = m_toVolume;
		prvntDrag = true;
	}

	ImGui::SetNextItemWidth(m_drawSize.x);
	if (ImGui::SliderFloat("speed", &m_playSpeed, 0.0f, 6.f)) {
		setPlaySpeed(m_playSpeed);
		prvntDrag = true;
	}

		

	if (ImGui::Checkbox("looping", &m_isLooping)) {
		m_soundRoomNode->loop(m_isLooping);
	}

	ImGui::SameLine();
	if (ImGui::Checkbox("waveform", &m_showWaveform)) {
			
	}

	std::string length = "length: "+ std::to_string(m_soundRoomNode->getSeconds());
	ImGui::Text(length.c_str());

		
	ImGui::PushID(1);
	ImGui::BeginDisabled();
	ImGui::SliderFloat("", &m_playPosition, 0.0f, 1.0f);
	ImGui::EndDisabled();
	ImGui::PopID();

	ImGui::PushID(2);
	if (ImGui::SliderFloat("", &m_fadeInPosition, 0.0f, 1.0f)) {
		if (m_fadeInPosition > m_fadeOutPosition)
			m_fadeInPosition = m_fadeOutPosition;
		if (m_soundRoomNode)
			m_soundRoomNode->setFadeIn(m_fadeInPosition);
		prvntDrag = true;
	}
	ImGui::PopID();

	ImGui::PushID(3);
	if (ImGui::SliderFloat("", &m_fadeOutPosition, 0.0f, 1.0f)) {
		if (m_fadeOutPosition < m_fadeInPosition)
			m_fadeOutPosition = m_fadeInPosition;
		if (m_soundRoomNode)
			m_soundRoomNode->setFadeOut(m_fadeOutPosition);
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

void act::proc::Audio3DPlayerTimestretchProcNode::onTrigger(bool event) {
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

bool act::proc::Audio3DPlayerTimestretchProcNode::play()
{
	bool wasPlaying = m_isPlaying;
	m_soundRoomNode->play();
	m_isPlaying = true;
	return !wasPlaying;
}

bool act::proc::Audio3DPlayerTimestretchProcNode::stop()
{
	bool wasPlaying = m_isPlaying;
	m_isPlaying = false;
	m_soundRoomNode->stop();
	//m_playPosition = 0.0f;
	return !wasPlaying;
}

void act::proc::Audio3DPlayerTimestretchProcNode::setPlaySpeed(float speed)
{
	m_playSpeed = speed;
	m_soundRoomNode->setSpeed(speed);
}

void act::proc::Audio3DPlayerTimestretchProcNode::loadSound(std::filesystem::path path) {
	if(path != "") {
		try {
			m_soundRoomNode = m_audioMgr->createSoundFile(m_3DPosition, path, 0.2f, path.filename().string(), true);
			
			m_soundRoomNode->setVolume(m_toVolume);
			m_soundRoomNode->setFadeIn(m_fadeInPosition);
			m_soundRoomNode->setFadeOut(m_fadeOutPosition);
			set3DPosition(m_3DPosition);
			

			auto waveform = WaveformPlot();
			auto buf = m_soundRoomNode->getBufferPlayer()->getBuffer();
			waveform.load(buf, Rectf(vec2(0, 0), m_drawSize));

			gl::Fbo::Format format;
			format.setSamples( 4 );
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
			m_length = m_soundRoomNode->getSeconds();

			m_bufferPort->send(buf);			
		} catch(...) {
			// it's not a sound
		}
		m_soundRoomNode->stop();
	}
}

ci::Json act::proc::Audio3DPlayerTimestretchProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["path"]			= m_path;
	json["isPlaying"]		= m_isPlaying;
	json["showWaveform"]	= m_showWaveform;
	json["isCollapsed"]		= m_isCollapsed;
	json["toVolume"]		= m_toVolume;
	json["volume"]			= m_volume.value();
	json["looping"]			= m_isLooping;
	json["fadeInPosition"]  = m_fadeInPosition;
	json["fadeOutPosition"] = m_fadeOutPosition;
	json["playPosition"]	= m_playPosition;
	json["length"]			= m_length;
	vec3 pos;
	if (m_soundRoomNode) {
		pos = m_soundRoomNode->getPosition();
	}
	else {
		pos = vec3(0.0, 0.0, 0.0);
	}
	
	json["x"] = pos[0];
	json["y"] = pos[1];
	json["z"] = pos[2];
	return json;
}

void act::proc::Audio3DPlayerTimestretchProcNode::fromParams(ci::Json json) {
	// get new values for comparison
	bool shallPlay;
	float newX;
	float newY;
	float newZ;

	if (util::setValueFromJson(json, "path", m_path)) {
		init();
	};
	if (util::setValueFromJson(json, "isPlaying", shallPlay)) {
		if (shallPlay != m_isPlaying) {
			onTrigger(true);
		};
		
	};

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
		if(m_soundRoomNode)
			m_soundRoomNode->setVolume(m_volume.value());
	};
	if (util::setValueFromJson(json, "fadeInPosition", m_fadeInPosition)) {
		if (m_soundRoomNode)
			m_soundRoomNode->setFadeIn(m_fadeInPosition);
	};
	if (util::setValueFromJson(json, "fadeOutPosition", m_fadeOutPosition)) {
		if (m_soundRoomNode)
			m_soundRoomNode->setFadeOut(m_fadeOutPosition);
	};
	util::setValueFromJson(json, "looping", m_isLooping);
	util::setValueFromJson(json, "showWaveform", m_showWaveform);
	util::setValueFromJson(json, "isCollapsed", m_isCollapsed);
}

void act::proc::Audio3DPlayerTimestretchProcNode::set3DPosition(vec3 position)
{
	m_3DPosition = position;
	if(m_soundRoomNode)
		m_soundRoomNode->setPosition(position);
}
