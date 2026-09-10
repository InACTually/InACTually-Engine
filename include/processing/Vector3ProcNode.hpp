/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2026 InACTually Community
	Licensed under the MIT License.
	See LICENSE file in the project root for full license information.

	This file is created and substantially modified: 2026

	contributors:
	Lars Engeln - mail@lars-engeln.de
*/

#pragma once

#include "ProcNodeBase.hpp"


namespace act {
	namespace proc {

		class Vector3ProcNode : public ProcNodeBase
		{
		public:
			Vector3ProcNode();
			~Vector3ProcNode();

			PROCNODECREATE(Vector3ProcNode);

			void setup(act::room::RoomManagersRef roomMgrs)	override;
			void update()			override;
			void draw()				override;

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			OutputPortRef<glm::vec3>	m_outPort;
			glm::vec3 m_vector;

		}; using Vector3ProcNodeRef = std::shared_ptr<Vector3ProcNode>;

	}
}
