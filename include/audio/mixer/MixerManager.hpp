
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

#pragma once

#include <memory>
#include <vector>

#include "cinder/audio/audio.h"

#include "mixer/MixerBase.hpp"

#include "audio/SpeakerRoomNode.hpp"
#include "audio/SubwooferRoomNode.hpp"
#include "audio/SoundRoomNode.hpp"

namespace act {
	namespace aio {

		enum MixerType {
			MT_DISTANCE = 0,
			MT_NEAREST,
			MT_2D,
			MT_3D
		};

		class MixerManager {
		public:
			MixerManager();
			~MixerManager();

			static std::shared_ptr<MixerManager> create() { return std::make_shared<MixerManager>(); };

			void update();

			void clear();
			void connectSound(act::room::SoundRoomNodeRef sound);
			void configure(std::vector<act::room::SpeakerRoomNodeRef> speakers, std::vector<act::room::SubwooferRoomNodeRef> subwoofer, std::vector<act::room::SoundRoomNodeRef> sounds);

			void drawSettings();

			MixerType getMixerType() { return (MixerType)m_mixerSelection; };
			void setMixer(MixerType type);

		protected:
			act::aio::MixerRef						m_mixer;

			std::vector<act::room::SpeakerRoomNodeRef>		m_speakers;
			std::vector<act::room::SubwooferRoomNodeRef>	m_subwoofers;
			std::vector<act::room::SoundRoomNodeRef>		m_sounds;

			int											m_mixerSelection;
			std::vector<std::string>					m_mixerNames;
		};
		using MixerManagerRef = std::shared_ptr<MixerManager>;
	}
}