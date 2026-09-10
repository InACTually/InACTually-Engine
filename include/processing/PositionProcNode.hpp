
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021–2025 Lars Engeln, Fabian Töpfer
	Copyright (c) 2025 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2021-2024

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "ProcNodeBase.hpp"
#include "position/PositionRoomNode.hpp"
 

namespace act {
	namespace proc {

		class PositionProcNode : public ProcNodeBase
		{
		public:
			PositionProcNode();
			~PositionProcNode();

			PROCNODECREATE(PositionProcNode);

			void setup(act::room::RoomManagersRef)	override;
			void update()							override;
			void draw()								override;

			ci::Json toParams()						override;
			void fromParams(ci::Json json)			override;

		private:
			InputPortRef<float>			m_tPort;
			OutputPortRef<glm::vec3>		m_positionPort;
			glm::vec3					m_pos;
			float						m_t;

			room::PositionRoomNodeRef	m_positionRoomNode;

			room::PositionManagerRef		m_posMgr;

		}; using PositionProcNodeRef = std::shared_ptr<PositionProcNode>;

	}
}