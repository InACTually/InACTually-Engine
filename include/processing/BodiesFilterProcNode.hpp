
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


namespace act {
	namespace proc {

		class BodiesFilterProcNode : public ProcNodeBase
		{
		public:
			BodiesFilterProcNode();
			~BodiesFilterProcNode();

			PROCNODECREATE(BodiesFilterProcNode);

			void setup();
			void update()			override;
			void draw()				override;

			void drawPickPosition();

			void onBodies(room::BodyRefList event);

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			room::BodyRefList m_bodies;

			std::vector<std::string> m_bodySelection = { "Position", "First", "Second", "Third", "Fourth", "Last" };
			int m_currentBody = 0;

			int m_numberOfBodies = 0;
			glm::vec3 m_nearestToPosition;
			float m_nearestToPositionDistance;
			glm::vec3 m_currentPosition;

			bool m_isBody;

			OutputPortRef<room::BodyRef> m_bodyOutPort;

			act::room::BodyRef findNearestBody(room::BodyRefList bodies);

		}; using BodiesFilterProcNodeRef = std::shared_ptr<BodiesFilterProcNode>;
	}
}