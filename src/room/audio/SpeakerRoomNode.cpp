
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022-2023, 2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "roompch.hpp"
#include "audio/SpeakerRoomNode.hpp"

#include "cinder/audio/Param.h"



act::room::SpeakerRoomNode::SpeakerRoomNode(int channel, ci::vec3 position, float radius, std::string name = "speaker")
	: AudioRoomNodeBase(name, position, ci::vec3(0.0f), radius), m_channel(channel)
{
	m_isFixed = false;

	setTriMesh(ci::TriMesh::create(ci::geom::Cylinder()));

	setupAudioNodes();
}

act::room::SpeakerRoomNode::~SpeakerRoomNode()
{
}

void act::room::SpeakerRoomNode::setup()
{
}

void act::room::SpeakerRoomNode::update()
{
}

void act::room::SpeakerRoomNode::draw()
{
	ci::gl::ScopedColor color(ci::Color(0.5f, 0.5f, 0.5f));

	enableStatusColor();

	// Draw the mesh.
	{
		//ci::gl::ScopedColor color(ci::Color::white());

		ci::gl::ScopedModelMatrix model;
		ci::gl::multModelMatrix(m_transform);

		//ci::gl::drawCube(getPosition(), ci::vec3(getRadius()) * ci::vec3(1.0f, 1.1f, 1.0f));
		m_mesh->draw();
	}
}

ci::audio::NodeRef act::room::SpeakerRoomNode::getIn()
{
	return m_add;
}

ci::audio::NodeRef act::room::SpeakerRoomNode::getOut()
{
	return m_gain;
}

void act::room::SpeakerRoomNode::setupAudioNodes()
{
	auto ctx = ci::audio::Context::master();

	m_gain = ctx->makeNode(new ci::audio::GainNode(ci::audio::decibelToLinear(m_volume)));

	m_add = ctx->makeNode(new ci::audio::AddNode());
	//m_eq  = ctx->makeNode(new EqualizerNode(ci::audio::Node::Format().channels(1)));
	
	m_noiseVolume = 70;
	m_noiseGain = ctx->makeNode(new ci::audio::GainNode(ci::audio::decibelToLinear(m_noiseVolume)));
	m_noise = ctx->makeNode(new ci::audio::GenNoiseNode());
	m_noise->disable();

	m_sineVolume = 70;
	m_sineGain = ctx->makeNode(new ci::audio::GainNode(ci::audio::decibelToLinear(m_sineVolume)));
	m_sine = ctx->makeNode(new ci::audio::GenSineNode());
	m_sine->disable();

	disconnectExternals();
}

void act::room::SpeakerRoomNode::disconnectExternals()
{
	getOut()->disconnectAll();
	m_noise >> m_noiseGain >> getIn();
	m_sine  >> m_sineGain  >> getIn();
	getIn() >> /*m_eq >>*/ getOut();
}

void act::room::SpeakerRoomNode::locate(bool isLocating)
{
	m_noise->setEnabled(isLocating);
}

void act::room::SpeakerRoomNode::setNoiseVolume(float db, float rampTime) {
	ci::app::timeline().apply(&m_noiseVolume, db, rampTime).updateFn([&]() {
		m_noiseGain->setValue(audio::decibelToLinear(m_noiseVolume));
	});
}

void act::room::SpeakerRoomNode::calibrate(std::string type, float duration, float volume)
{
	if (type == "sine") {
		m_sineVolume = volume;
		m_sineGain->setValue(ci::audio::decibelToLinear(m_sineVolume));
		m_sineFreq = 1;
		m_sine->setFreq(m_sineFreq);
		m_sine->setEnabled(true);
		ci::app::timeline().apply(&m_sineFreq, ci::audio::master()->getSampleRate() * 0.5f, duration).updateFn([&]() {
			m_sine->setFreq(m_sineFreq);
		});

		ci::app::timeline().apply(&m_calibrateProgress, 0.0f, 0.1f);
		ci::app::timeline().appendTo(&m_calibrateProgress, 1.0f, duration).finishFn([&]() {
			m_sine->setEnabled(false);
		});
	}
	else if (type == "white") {
		m_noiseVolume = volume;
		m_noiseGain->setValue(ci::audio::decibelToLinear(m_noiseVolume));
		m_noise->setEnabled(true);

		ci::app::timeline().apply(&m_calibrateProgress, 0.0f, 0.1f);
		ci::app::timeline().appendTo(&m_calibrateProgress, 1.0f, duration).finishFn([&]() {
			m_noise->setEnabled(false);
		});
	}
}

void act::room::SpeakerRoomNode::drawSpecificSettings()
{
	if (ImGui::InputInt("Channel", &m_channel, 1, 1)) {

	}
	if (ImGui::DragFloat("Volume", &m_volume.value(), 0.1f, 0.0f, 120.0f)) {
		setVolume(m_volume, 0.1f);
	}

	ImGui::Separator();
	if (ImGui::DragFloat("NoiseVolume", &m_noiseVolume.value(), 0.1f, 0.0f, 120.0f)) {
		m_noiseGain->setValue(ci::audio::decibelToLinear(m_noiseVolume));
	}
	if (ImGui::SmallButton("locate")) {
		locate(!m_noise->isEnabled());
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("sweep")) {
		calibrate("sine", 10, m_noiseVolume);
	}

}

ci::Json act::room::SpeakerRoomNode::toParams() {
	ci::Json json = ci::Json::object();
	json["channel"]	= m_channel;
	json["volume"]	= m_volume.value();
	return json;
}

void act::room::SpeakerRoomNode::fromParams(ci::Json json) {
	util::setValueFromJson(json, "channel", m_channel);
	float volume = 100;
	if (util::setValueFromJson(json, "volume", volume)) {
		setVolume(volume, 0.1f);
	};
}