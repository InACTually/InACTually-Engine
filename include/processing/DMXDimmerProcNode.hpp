
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
#include "room/dmx/DimmerRoomNode.hpp"


namespace act {
	namespace proc {

		class DMXDimmerProcNode : public ProcNodeBase
		{
		public:
			DMXDimmerProcNode();
			~DMXDimmerProcNode();

			PROCNODECREATE(DMXDimmerProcNode);

			void update()			override;
			void draw()				override;
			void setup(act::room::RoomManagersRef roomMgrs) override;

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			OutputPortRef<float>	m_dimPort;
			float m_dim;
			room::DimmerRoomNodeRef	m_dimmer;
			int		m_selectedDimmer;
			room::DMXManagerRef		m_dmxMgr;

		}; using DMXDimmerProcNodeRef = std::shared_ptr<DMXDimmerProcNode>;

	}
}