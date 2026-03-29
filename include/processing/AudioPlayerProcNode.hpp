
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

#pragma once

#include "ProcNodeBase.hpp"

#include "cinder/audio/audio.h"
#include "../3rd/AudioDrawUtils.h"

#include "cinder/Timeline.h"

using namespace ci;
using namespace ci::app;


namespace act {
	namespace proc {

		class AudioPlayerProcNode : public ProcNodeBase
		{
		public:
			AudioPlayerProcNode();
			~AudioPlayerProcNode();

			PROCNODECREATE(AudioPlayerProcNode);

			void setup(act::room::RoomManagers roomMgrs)			override;
			void update()			override;
			void draw()				override;

			void onTrigger(bool event);

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;
			
		private:
			void init();
			room::AudioManagerRef			m_audioMgr;

			std::string						m_path;
			ci::audio::SourceFileRef		m_sourceFile;
			ci::audio::BufferRef			m_buffer;
			ci::audio::BufferPlayerNodeRef	m_bufferPlayer;
			ci::audio::GainNodeRef			m_gain;
			int								m_channel;
			ci::audio::ChannelRouterNodeRef	m_router;
			aio::TimeStrechingNodeRef		m_stretch;
			bool							m_isStretching;
			float							m_playSpeed;
			ci::Anim<float>					m_volume;
			float							m_toVolume;

			OutputPortRef<audio::NodeRef>	m_audioOut;
			OutputPortRef<bool>				m_isPlayingOut;

			bool							m_playEvent;
			bool							m_stopEvent;

			void route();

			void setPlaySpeed(float speed);

			void ramp(float toVolume);
			void fadeIn();
			void fadeOut();
			float							m_fadeTime;
			bool							m_isFading;

			bool							m_isLooping;
			bool							m_isResuming;
			bool							m_normalized;
			
			WaveformPlot					m_waveform;
			gl::Texture2dRef				m_waveformTex;
			float							m_playPosition;

			int								m_falseCount;
			int								m_countFalseUpTo;
			
			void loadSound(std::string path);
			bool m_isOpenDialog;
			bool m_isPlaying;
		};

		using AudioPlayerProcNodeRef = std::shared_ptr<AudioPlayerProcNode>;

	}
}