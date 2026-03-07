
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2022

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#include "mixer/MixerManager.hpp"

#include "mixer/DistanceMixer.hpp"
#include "mixer/NearestMixer.hpp"
#include "mixer/Mixer2d.hpp"
#include "mixer/Mixer3d.hpp"

#include "cinder/CinderImGui.h"

act::aio::MixerManager::MixerManager() {
	m_mixerNames.push_back("Distance");
	m_mixerNames.push_back("Nearest");
	m_mixerNames.push_back("2D");
	m_mixerNames.push_back("3D");

	setMixer(MT_DISTANCE);
}

act::aio::MixerManager::~MixerManager() {
}

void act::aio::MixerManager::update() {
	m_mixer->update();
}

void act::aio::MixerManager::clear() {
	m_speakers.clear();
	m_subwoofers.clear();
	m_sounds.clear();
	m_mixer->clear();
}

void act::aio::MixerManager::connectSound(room::SoundRoomNodeRef sound) {
	m_sounds.push_back(sound);
	m_mixer->connectSound(sound, m_speakers, m_subwoofers);
}

void act::aio::MixerManager::configure(std::vector<room::SpeakerRoomNodeRef>   speakers,
                                     std::vector<room::SubwooferRoomNodeRef> subwoofers,
                                     std::vector<room::SoundRoomNodeRef>	  sounds) {
	m_speakers.clear();
	for (auto&& speaker : speakers) {
		if (speaker->getChannel() < ci::audio::master()->getOutput()->getNumChannels()) {
			m_speakers.push_back(speaker);
		}
	}
	m_subwoofers = subwoofers;
	m_sounds = sounds;

	//setMixer((MixerType)m_mixerSelection);
	m_mixer->configure(m_speakers, m_subwoofers, m_sounds);
}

void act::aio::MixerManager::drawSettings() {
	//ImGui::Text("MixerSettings");
	if (ImGui::Combo("Mixer", &m_mixerSelection, m_mixerNames)) {
		setMixer(static_cast<MixerType>(m_mixerSelection));
	}
}

void act::aio::MixerManager::setMixer(MixerType type) {
	
	m_mixerSelection = type;
	aio::MixerRef mixer;
	switch (type) {
		case MT_DISTANCE:
			mixer = DistanceMixer::create();
			break;
		case MT_NEAREST:
			mixer = NearestMixer::create();
			break;
		case MT_2D:
			mixer = Mixer2d::create();
			break;
		case MT_3D:
			mixer = Mixer3d::create();
			break;

	}

	if (!mixer)
		return;
	
	if (m_mixer)
		m_mixer->clear();
	m_mixer = mixer;
	configure(m_speakers, m_subwoofers, m_sounds);
}
