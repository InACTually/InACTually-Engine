
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

#pragma once

#include "ProcNodeBase.hpp"

#include "cinder/audio/audio.h"
#include "../3rd/AudioDrawUtils.h"

#include "cinder/Timeline.h"

using namespace ci;
using namespace ci::app;


namespace act {
	namespace proc {

		class Audio3DPlayerProcNode : public ProcNodeBase
		{
		public:
			Audio3DPlayerProcNode();
			~Audio3DPlayerProcNode();

			PROCNODECREATE(Audio3DPlayerProcNode);

			void setup(act::room::RoomManagers roomMgrs)			override;
			void update()			override;
			void draw()				override;

			void onTrigger(bool event);
			void onReset(bool event);
			bool play();
			bool stop();

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;
			
		private:
			void init();
			room::AudioManagerRef			m_audioMgr;

			std::vector<std::string>		m_paths;
			ci::Anim<float>					m_volume;
			float							m_toVolume;

			OutputPortRef<float>			m_currentVolumePort;

			float							m_fadeInPosition;
			float							m_fadeOutPosition;

			bool							m_isLooping;
			bool							m_noTimestretch;

			vec3							m_3DPosition;
			void							set3DPosition(vec3 position);
			std::vector<room::SoundFileRoomNodeRef>	 m_soundRoomNodes;
			OutputPortRef<ci::audio::BufferRef>	m_bufferPort;
			
			float							m_playSpeed;

			WaveformPlot					m_waveform;
			gl::Texture2dRef				m_waveformTex;
			float							m_playPosition;
			OutputPortRef<float>			m_playPosPort;
			float							m_length;

			OutputPortRef<bool>				m_finishedPort;

			int								m_falseCount;
			int								m_countFalseUpTo;

			void setPlaySpeed(float speed);

			void loadSound(std::filesystem::path path);
			bool m_isOpenDialog;
			bool m_isAdding = false;
			bool m_hasChannelSplit = false;
			bool m_isPlaying;
			bool m_isCollapsed  = false;
			bool m_showWaveform = false;
		};

		using Audio3DPlayerProcNodeRef = std::shared_ptr<Audio3DPlayerProcNode>;

	}
}