
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2024-2025

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "ProcNodeBase.hpp"

#include "cinder/audio/audio.h"

#include "cinder/Timeline.h"


namespace act {
	namespace proc {

		class Audio3DProcNode : public ProcNodeBase
		{
		public:
			Audio3DProcNode();
			~Audio3DProcNode();

			PROCNODECREATE(Audio3DProcNode);

			void setup(act::room::RoomManagersRef roomMgrs)			override;
			void update()			override;
			void draw()				override;

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;
			
		private:
			void init();
			room::AudioManagerRef				m_audioMgr;

			ci::Anim<float>						m_volume;
			float								m_toVolume;
			float								m_currentLoudness;

			glm::vec3								m_3DPosition;
			void								set3DPosition(glm::vec3 position);
			room::SoundRoomNodeRef				m_soundRoomNode;

			OutputPortRef<ci::audio::NodeRef>	m_outPort;
			
			bool								m_showVisual = false;
		};

		using Audio3DProcNodeRef = std::shared_ptr<Audio3DProcNode>;

	}
}