
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2025

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "procpch.hpp"
#include "AudioPlayerProcNode.hpp"
#include "cinder/gl/Fbo.h"
#include "cinder/audio/Param.h"

act::proc::AudioPlayerProcNode::AudioPlayerProcNode() : ProcNodeBase("AudioPlayer") {

	m_playPosition	= 0.0f;
	m_path			= "";
	m_playSpeed		= 1.0f;
	m_volume		= 90.0f;
	m_toVolume		= 90.0f;
	m_isOpenDialog	= false;
	m_isPlaying		= false;
	m_isStretching  = false;
	m_isLooping		= false;
	m_isResuming	= false;
	m_fadeTime		= 0.40f;
	m_isFading		= false;
	m_falseCount	= 0;
	m_countFalseUpTo = 5;
	m_normalized	= false;
	m_channel		= 0;

	m_playEvent = false; // will be altered by front end
	m_stopEvent = false; // will be altered by front end

	m_drawSize	= ivec2(400, 150);
	
	auto trigger = createBoolInput("fire", [&](bool event) { this->onTrigger(event); });
	auto gain = createNumberInput("gain", [&](float event) { m_gain->setValue(event); m_volume = audio::linearToDecibel(event); });
	auto speed = createNumberInput("speed", [&](float event) { setPlaySpeed(event); });

	
	//m_imagePort = createImageOutput("pass-through image");

	m_audioOut = createAudioNodeOutput("audioOut");
	// This output port is used to let the Frontend Observe, if the Player plays
	m_isPlayingOut = createBoolOutput("isPlayingOut");
	//-----------------------------------------------------------------------------
	auto ctx = audio::Context::master();
	//ctx->disable();

	m_bufferPlayer	= ctx->makeNode(new audio::BufferPlayerNode());
	m_gain			= ctx->makeNode(new audio::GainNode(audio::decibelToLinear(m_volume.value())));
	m_router		= ctx->makeNode(new audio::ChannelRouterNode());
	m_bufferPlayer >> m_gain;
	route();
}

act::proc::AudioPlayerProcNode::~AudioPlayerProcNode() {
	if(m_isPlaying) {
		m_bufferPlayer->stop();
		if(m_stretch)
			m_stretch->pause();
		m_gain->disconnectAll();
	}
}

void act::proc::AudioPlayerProcNode::setup(act::room::RoomManagers roomMgrs) {
	m_audioMgr = roomMgrs.audioMgr;
}

void act::proc::AudioPlayerProcNode::init() {
	loadSound(m_path);
	m_gain->setValue(audio::decibelToLinear(m_volume));
}

void act::proc::AudioPlayerProcNode::update() {
	if (m_isOpenDialog) {
		m_isOpenDialog = false;
		m_path = ci::app::getOpenFilePath().string();
		loadSound(m_path);
	}

	if (m_bufferPlayer && m_bufferPlayer->getBuffer()) {
		m_playPosition = m_bufferPlayer->getReadPosition() / m_bufferPlayer->getNumFrames();
		if (!m_isFading && m_bufferPlayer->getNumSeconds() >= (m_bufferPlayer->getReadPositionTime() + m_fadeTime)) {
			fadeOut();
		}
	}

	if (m_bufferPlayer->isEof()) {
		m_playPosition = 0.0f;
		if (!m_isLooping)
			m_isPlaying = false;
	}
	
	if (m_normalized == true) {
		audio::dsp::normalize(m_buffer->getData(), m_buffer->getSize());
	}
	
	if (m_bufferPlayer && m_playEvent) { // Event from Front End
		onTrigger(true);
		m_playEvent = false;
	}

	if (m_bufferPlayer && m_stopEvent) {
		onTrigger(true);
		m_stopEvent = false;
	}

	if (m_bufferPlayer && m_isPlaying) { 
		m_audioOut->send(m_bufferPlayer);
	} 
	
	m_isPlayingOut->send(m_isPlaying);
}

void act::proc::AudioPlayerProcNode::draw() {
	beginNodeDraw();

	if(ImGui::Button("load")) {
		m_isOpenDialog = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("play")) {
		onTrigger(true);
	}
	ImGui::SameLine();
	if (ImGui::Button("stop")) {
		onTrigger(false);
	}

	bool prvntDrag = false;

	ImGui::SameLine();
	if(ImGui::InputInt("channel", &m_channel, 1, 2)) {
		m_channel = std::clamp(m_channel, 0, 127);
		route();
		prvntDrag = true;
	}
	
	ImGui::SetNextItemWidth(m_drawSize.x);
	if (ImGui::SliderFloat("volume", &m_toVolume, 0.0f, 120.f)) {
		//m_gain->setValue(audio::decibelToLinear(m_volume));
		ramp(m_toVolume);
		prvntDrag = true;
	}
	ImGui::SetNextItemWidth(m_drawSize.x);
	if (ImGui::SliderFloat("speed", &m_playSpeed, 0.0f, 4.f)) {
		setPlaySpeed(m_playSpeed);
		prvntDrag = true;
	}


	preventDrag(prvntDrag);
	
	if (ImGui::Checkbox("looping", &m_isLooping)) {
		m_bufferPlayer->setLoopEnabled(m_isLooping);
	}
	ImGui::SameLine();
	ImGui::Checkbox("resuming", &m_isResuming);
	ImGui::SameLine();
	ImGui::Checkbox("normalize", &m_normalized);
	
	if(m_waveformTex)
		ImGui::Image(m_waveformTex, m_waveformTex->getSize(), vec2(0, 1), vec2(1, 0));

	ImGui::SetNextItemWidth(m_drawSize.x);
	ImGui::InputInt("count Trigger", &m_countFalseUpTo, 0, 20);

	endNodeDraw();
}

void act::proc::AudioPlayerProcNode::onTrigger(bool event) {
	if (event)
		m_falseCount = 0;
	else {
		if(m_falseCount < m_countFalseUpTo)
			m_falseCount++;
	}
	
	if (m_falseCount < m_countFalseUpTo && !m_isPlaying && m_bufferPlayer->getBuffer()) {
		m_isPlaying = true;
		fadeIn();
		if (!m_isStretching) {
			m_bufferPlayer->seek(m_playPosition * m_bufferPlayer->getNumFrames());
			m_bufferPlayer->start();
		}
		else {
			m_stretch->play();
		}
		
	}
	else if (m_falseCount >= m_countFalseUpTo && m_isPlaying) {
			if (!m_isResuming) {
				m_playPosition = 0.0f;
			}
			m_isPlaying = false;
			fadeOut();
	}
}

void act::proc::AudioPlayerProcNode::loadSound(std::string path) {
	if(path == "")
		return; 

	try {
		m_sourceFile = ci::audio::load(loadFile(path), audio::Context::master()->getSampleRate());
	}
	catch (...) {
		// it's not a sound
		return;
	}

	m_gain->disconnectAll();

	if (!m_isStretching) {
		m_buffer = m_sourceFile->loadBuffer();
		m_bufferPlayer->setBuffer(m_buffer);

		m_normalized = false;
		m_bufferPlayer->stop(); //required to push 0s to outputPort, otherwise the track has to be silent for a few frames
		m_isPlaying = false;
		m_bufferPlayer->seek(0);

		auto ctx = audio::Context::master();
		m_bufferPlayer >> m_gain >> ctx->getOutput();

		auto waveform = WaveformPlot();
		waveform.load(m_buffer, Rectf(vec2(0, 0), m_drawSize));

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
	}
	else {
		auto ctx = audio::Context::master();
		m_stretch = ctx->makeNode(new aio::TimeStretchingNode(m_sourceFile, 120));
		m_stretch >> m_gain >> ctx->getOutput();
	}

}

ci::Json act::proc::AudioPlayerProcNode::toParams() {
	ci::Json json = ci::Json::object();
	json["path"]		= m_path;
	json["tovolume"]	= m_toVolume;
	json["volume"]		= m_volume.value();
	json["stretching"]  = m_isStretching;
	json["playSpeed"]	= m_playSpeed;
	json["looping"]		= m_isLooping;
	json["resuming"]	= m_isResuming;
	json["startEvent"]	= m_playEvent;
	json["stopEvent"]	= m_stopEvent;
	return json;
}

void act::proc::AudioPlayerProcNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "path", m_path);
	util::setValueFromJson(json, "volume", m_volume.value());
	util::setValueFromJson(json, "tovolume", m_toVolume);
	util::setValueFromJson(json, "stretching", m_isStretching);
	util::setValueFromJson(json, "playSpeed", m_playSpeed);
	util::setValueFromJson(json, "looping", m_isLooping);
	util::setValueFromJson(json, "resuming", m_isResuming);
	util::setValueFromJson(json, "startEvent", m_playEvent);
	util::setValueFromJson(json, "stopEvent", m_stopEvent);

	init();
}

void act::proc::AudioPlayerProcNode::route()
{
	m_router->disconnectAll();
	auto ctx = audio::Context::master();

	int inputChannel = m_buffer->getNumChannels();
	int outputChannel = ctx->getOutput()->getNumChannels();
	
	for(int in = 0; in < inputChannel; in++) {
		int out = (in + m_channel);
		if (out < outputChannel)
			m_gain >> m_router->route(in, out) >> ctx->getOutput();
	}
}

void act::proc::AudioPlayerProcNode::setPlaySpeed(float speed)
{
	m_playSpeed = speed;
	//if (m_playSpeed != 1.0f) {
		if (!m_isStretching) {
			m_isStretching = true;
			init();
		}
		m_stretch->setPlaybackSpeed(m_playSpeed);
	/* }
	else if (m_isStretching) {
		m_isStretching = false;
		init();
	}*/
}

void act::proc::AudioPlayerProcNode::ramp(float toVolume) {
	ci::app::timeline().apply(&m_volume, toVolume, 2).updateFn([&]() {
		m_toVolume = m_volume;
		m_gain->setValue(audio::decibelToLinear(m_volume));
	});
}

void act::proc::AudioPlayerProcNode::fadeIn() {
	m_volume = 0.0f;
	m_gain->setValue(audio::decibelToLinear(m_volume));
	ci::app::timeline().apply(&m_volume, m_toVolume, m_fadeTime, EaseInOutSine()).updateFn([&]() {
		m_gain->setValue(audio::decibelToLinear(m_volume));
	});
}

void act::proc::AudioPlayerProcNode::fadeOut() {
	m_isFading = true;
	ci::app::timeline().apply(&m_volume, 0.01f, m_fadeTime*10, EaseInOutSine()).updateFn([&]() {
		m_gain->setValue(audio::decibelToLinear(m_volume));
		});
	ci::app::timeline().appendTo(&m_volume, 0.01f, 0.01f).finishFn([&]() {
		if (m_isLooping)
			fadeIn();
		else {
			if (!m_isStretching)
				m_bufferPlayer->stop();
			else
				m_stretch->pause();
			m_isPlaying = false;
		}
		m_isFading = false;
	});
}