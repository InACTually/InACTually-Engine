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
#include "MatListener.hpp"

#include "lidar/LidarManager.hpp"

using namespace ci;
using namespace ci::app;

namespace act {
	namespace proc {

		class LidarProcNode : public ProcNodeBase
		{
		public:
			LidarProcNode();
			~LidarProcNode();

			PROCNODECREATE(LidarProcNode);

			void setup(act::room::RoomManagers roomMgrs)			override;
			void update()			override;
			void draw()				override;

			ci::Json toParams() override;
			void fromParams(ci::Json json) override;

		private:
			act::room::LidarManagerRef		m_lidarMgr;

			bool							m_show;

			OutputPortRef<number>			m_movementOutputPort;
			OutputPortRef<number>			m_blobAmountOutputPort;
			ImageOutputPortRef				m_imageOutputPort;

		};

		using LiDARProcNodeRef = std::shared_ptr<LidarProcNode>;

	}
}