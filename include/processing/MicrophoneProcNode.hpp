
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "ProcNodeBase.hpp"
#include "MatListener.hpp"

#include "audio/AudioManager.hpp"
 

namespace act {
	namespace proc {

		class MicrophoneProcNode : public ProcNodeBase
		{
		public:
			MicrophoneProcNode();
			~MicrophoneProcNode();

			PROCNODECREATE(MicrophoneProcNode);

			void setup(act::room::RoomManagersRef roomMgrs)			override;
			void update()			override;
			void draw()				override;

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			act::room::AudioManagerRef		m_audioMgr;
			int								m_selectedInput = 0;
			room::MicrophoneRoomNodeRef		m_microphone;

			void							attachMic(room::MicrophoneRoomNodeRef mic);

			OutputPortRef<ci::audio::NodeRef>	m_audioNodeOutPort;

			bool							m_show;

			static bool						m_registered;
		};

		using MicrophoneProcNodeRef = std::shared_ptr<MicrophoneProcNode>;

	}
}