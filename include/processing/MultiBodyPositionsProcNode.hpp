
/*
	InACTually
	> interactive theater for actual acts
	> this file is part of the "InACTually Engine", a MediaServer for driving all technology

	Copyright (c) 2021�2025 Lars Engeln, Fabian T�pfer
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

using namespace std::chrono;

using namespace ci;
using namespace ci::app;

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

			vec3 onSkeleton(room::BodyRef event);
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

			OutputPortRef<vec3>	m_position1OutPort;
			OutputPortRef<vec3>	m_position2OutPort;
			OutputPortRef<vec3>	m_position3OutPort;
			OutputPortRef<vec3>	m_position4OutPort;
			std::vector<OutputPortRef<vec3>> m_positionPorts;

			OutputPortRef<float>	m_distance1OutPort;
			OutputPortRef<float>	m_distance2OutPort;
			OutputPortRef<float>	m_distance3OutPort;
			OutputPortRef<float>	m_distance4OutPort;
			std::vector<OutputPortRef<float>> m_distancePorts;

			static bool	m_registered;

		}; using MultiBodyPositionsProcNodeRef = std::shared_ptr<MultiBodyPositionsProcNode>;
	}
}