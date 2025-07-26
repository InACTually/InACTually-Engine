
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

using namespace ci;
using namespace ci::app;

namespace act {
	namespace proc {

		class MarkerProcNode : public ProcNodeBase
		{
		public:
			MarkerProcNode();
			~MarkerProcNode();

			PROCNODECREATE(MarkerProcNode);

			void setup(act::room::RoomManagers roomMgrs)			override;
			void update()			override;
			void draw()				override;

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			act::room::MarkerManagerRef m_markerMgr;
			int						m_selectedMarker = 0;

			OutputPortRef<vec3>		m_markerPositionOutPort;
			InputPortRef<vec3>		m_markerPositionInPort;

			void attachMarker();
		};

		using MarkerProcNodeRef = std::shared_ptr<MarkerProcNode>;

	}
}