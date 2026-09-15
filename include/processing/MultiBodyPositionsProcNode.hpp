
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

#include <chrono>


namespace act {
	namespace proc {

		class MultiBodyPositionsProcNode : public ProcNodeBase
		{
		public:
			MultiBodyPositionsProcNode();
			~MultiBodyPositionsProcNode();

			PROCNODECREATE(MultiBodyPositionsProcNode);

			void setup();
			void update()			override;
			void draw()				override;

			glm::vec3 onSkeleton(room::BodyRef event);
			void onBodies(room::BodyRefList event);

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:

			const char* m_jointSelection[4] = {
				"Torso",
				"Floor",
				"left Hand",
				"right Hand"
			};

			int m_currentJoint;

			std::vector<OutputPortRef<glm::vec3>>	m_positionPorts;
			std::vector<OutputPortRef<float>>		m_distancePorts;

			static bool	m_registered;

		}; using MultiBodyPositionsProcNodeRef = std::shared_ptr<MultiBodyPositionsProcNode>;
	}
}