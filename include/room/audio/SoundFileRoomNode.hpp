
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

#include "audio/SoundRoomNode.hpp"
#include "cinder/audio/GenNode.h"
#include "cinder/audio/audio.h"
#include "TimeStretchingNode.hpp"

#include <memory>

namespace act {
	namespace room {

		class SoundFileRoomNode : public SoundRoomNode {
		public:
			SoundFileRoomNode(ci::vec3 position, std::filesystem::path path, float radius, std::string name = "", bool noTimestretch = true);
			~SoundFileRoomNode();

			static std::shared_ptr<SoundFileRoomNode> create(ci::vec3 position, std::filesystem::path path, float radius, std::string name = "", bool noTimestretch = true) { return std::make_shared<SoundFileRoomNode>(position, path, radius, name, noTimestretch); };

			std::string		getName() { return m_name; };

			virtual void	setup()		override;
			virtual void	update()	override;
			virtual void	draw()		override;

			void			disconnectExternals() override;

			void play();
			void stop();
			void loop(bool isLooping = true);

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

			std::function<void()> finishedLoadingFn = []() {};
			std::function<void()> finishedPlayingFn = []() {};

			void loadFile(fs::path path);

			float getPlayPosition() { return  (float)(m_bufferPlayerNode->getReadPositionTime() / m_bufferPlayerNode->getNumSeconds()); };
			float getSeconds()		{ return  (float)(m_bufferPlayerNode->getNumSeconds()); };
			int getNumFrames()		{ return  (int)(m_bufferPlayerNode->getNumFrames()); };
			int getNumChannels()	{ return  (int)(m_bufferPlayerNode->getNumChannels());	};
			ci::audio::BufferPlayerNodeRef getBufferPlayer() { return m_bufferPlayerNode; };

			float getCurrentVolume() { return m_monitorNode->getVolume(); };

			void setFadeIn(float fadeInPosition)   { m_fadeInPosition = fadeInPosition; }
			void setFadeOut(float fadeOutPosition) { m_fadeOutPosition = fadeOutPosition; }

			void setSpeed(float speed);
			void setVolume(float volume, float rampDuration = 0.1f) override;

		private: 
			std::string m_name;

			ci::audio::BufferPlayerNodeRef		m_bufferPlayerNode;

			aio::TimeStrechingNodeRef			m_stretcherNode;
			bool m_noTimestretch = true;

			audio::MonitorNodeRef				m_monitorNode;

			bool m_isPlaying = false;
			bool m_isLooping = false;
			bool m_isLoadingFile = false;
			bool m_isStretching = false;

			float m_fadeInPosition = 0.0f;
			float m_fadeOutPosition = 1.0f;
			bool m_isFading = false;
			float m_targetVolume;
			float m_speed = 1.0f;

			void drawSpecificSettings() override;
		};
		using SoundFileRoomNodeRef = std::shared_ptr<SoundFileRoomNode>;
	}
}