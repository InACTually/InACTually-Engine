
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022-2023

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "roompch.hpp"
#include "audio/AudioRoomNodeBase.hpp"


act::room::AudioRoomNodeBase::AudioRoomNodeBase(std::string name, glm::vec3 position, glm::vec3 rotation, float radius)
	: RoomNodeBase(name, position, rotation, radius)
{
	m_volume = 100;
	m_gain = ci::audio::Context::master()->makeNode(new ci::audio::GainNode(ci::audio::decibelToLinear(m_volume)));

}

act::room::AudioRoomNodeBase::~AudioRoomNodeBase()
{
}

void act::room::AudioRoomNodeBase::setVolume(float volume, float rampDuration)
{
	if (rampDuration == 0.0f) {
		m_volume.stop();
		m_volume = volume;
		m_gain->setValue(ci::audio::decibelToLinear(m_volume));
		return;
	}
	ci::app::timeline().apply(&m_volume, volume, rampDuration).updateFn([&]() {
		m_gain->setValue(ci::audio::decibelToLinear(m_volume));
	});
}

void act::room::AudioRoomNodeBase::rampVolume(float volume, float rampDuration)
{
	ci::app::timeline().appendTo(&m_volume, volume, rampDuration).updateFn([&]() {
		m_gain->setValue(ci::audio::decibelToLinear(m_volume));
	});
}
