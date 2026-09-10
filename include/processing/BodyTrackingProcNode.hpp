
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2022-2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "ProcNodeBase.hpp"

#include "body/Body.hpp"
#include "RoomNodeBase.hpp"


namespace act {
	namespace proc {

		class BodyTrackingProcNode : public ProcNodeBase
		{
		public:
			BodyTrackingProcNode();
			~BodyTrackingProcNode();

			PROCNODECREATE(BodyTrackingProcNode);

			void setup(act::room::RoomManagersRef roomMgrs)			override;
			void update()			override;
			void draw()				override;

		private:
			int m_numberOfBodies;

			act::room::BodyTrackingManagerRef	m_bodyTrackingMgr;

			std::vector<std::string>			m_contributingDevices;

			room::BodyRefList					m_bodies;

			InputPortRef<room::BodyRefList>		m_bodiesInPort;
			OutputPortRef<room::BodyRefList>	m_bodiesOutPort;

		}; using BodyTrackingProcNodeRef = std::shared_ptr<BodyTrackingProcNode>;
	}
}